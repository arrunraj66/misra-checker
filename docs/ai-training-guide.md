# Step by step: upload a project, collect fixes, train a model

The plan: use the checker and its converter to produce *verified* fixes, turn
them into training data, fine-tune a code model in Google Colab, then run it
locally on the RX 6700 XT through Docker. The commands were checked on this
machine, except the Colab notebook and the GPU model container, which need
Colab and the downloaded model to try.

## Shortcut: do everything in Google Colab

`colab/misra_colab_pipeline.ipynb` runs the whole flow in one notebook: build the checker,
upload a project zip, find findings, let an AI (Claude API, or a local model on the Colab GPU) propose fixes,
gate them, and download `report.html`, `findings.csv` and `fixed_project.zip`. The same flow runs anywhere with
`scripts/run_project_pipeline.py PROJECT.zip --out OUT --provider-cmd ... --license ...`.
Open the notebook from GitHub in Colab (File > Open notebook > GitHub). Training (section 5 below) is the last part of the notebook.
It needs a few hundred accepted pairs and an `eval.jsonl` that is only written once enough pairs exist.

## 0. Put the code on GitHub

```bash
git push -u origin claude/stoic-tesla-a53igu
```
GitHub no longer accepts passwords for pushes. Sign in once with either
`gh auth login` (GitHub CLI), a personal access token entered when git asks for
the password, or an SSH key and the `git@github.com:...` remote URL. Then open
a pull request to `main` on github.com.

Do not commit customer code, licensed MISRA documents, keys or API tokens
(`.gitignore` already blocks the usual key files).

## 1. Start the application

```bash
docker compose up -d --build        # GUI at http://localhost:8765
```
If a host copy of the GUI is still running, stop it first (same port).

## 2. Upload and check your source code

In the GUI (http://localhost:8765):

1. **Whole project:** zip the folder that holds `main.c`, `uart.c`, `usb.c`,
   headers and bundled libraries, then click **Upload project (.zip)**.
2. Optionally enter **Defines** (for example `STM32F4 DEBUG=1`) and **Extra include dirs**
   for headers that live outside the include paths the tool adds automatically.
3. Click **Analyze project**. All `.c` files are analyzed together, so
   cross-file rules run. Click a finding to see the line; use the **Libraries**
   chip to hide findings inside `lib`/`vendor`/`drivers`-style folders.
4. A single file or a few files: paste code, use **Add files**, then **Analyze**
   (Ctrl+Enter).
5. **Report / CSV / JSON** export the findings.

Headers you do not have (vendor SDK headers) cause compile errors; the other
rules still run, and the errors are shown in a notice.

## 3. Generate verified fixes (the training data source)

The converter asks a model to fix one file, then accepts the fix only if the
file still compiles, the targeted findings are gone, nothing got worse and your
`--verify-cmd` passes. Accepted runs are what you train on.

```bash
# project copy under ./workspace/myproj, then:
docker compose run --rm misra python3 /opt/misra/scripts/make_compile_db.py /workspace/myproj

# one-off development licence (the image is built with the development key)
printf 'development-key-do-not-ship' > workspace/dev.key
python3 scripts/issue_license.py --customer me --expires 2027-12-31 \
    --features convert --key-file workspace/dev.key > workspace/me.lic

docker compose run --rm misra misra-checker convert \
  --compile-commands /workspace/myproj/compile_commands.json \
  --file /workspace/myproj/src/usb.c \
  --provider-cmd /opt/misra/scripts/ai_providers/claude_provider.py \
  --verify-cmd "true" --output-dir /workspace/out/usb --license /workspace/me.lic
```
Replace `true` with a command that builds and tests your project.

Choose the "teacher" model:

- **Claude** (`claude_provider.py`, needs `ANTHROPIC_API_KEY`): strongest, but your source
  code is sent to the API, so only use it with code you may share.
- **Local model** (`ollama_provider.py`, section 6): nothing leaves your PC; start
  with an off-the-shelf Qwen2.5-Coder-14B pulled into Ollama, then replace it
  with your fine-tuned model later.
- **No model** (`deterministic_provider.py`): fixes only rules 7.1 to 7.3.

The output folder gets `<file>.prompt.txt`, `<file>.proposed`, a `.patch` and
`audit.jsonl`. **Read every patch.** The gates do not prove behaviour is unchanged.

Run the converter over many files and many projects; each file gives at most one
pair per run. A few hundred pairs is a start, a few thousand is better, and rules
with few examples will stay weak.

## 4. Build the training file

```bash
python3 scripts/build_training_data.py workspace/out/usb workspace/out/uart \
    --dataset workspace/train.jsonl --eval workspace/eval.jsonl
```
Only runs whose last audit record is `proposed` with fewer findings are kept.
Re-running adds only new pairs. Spot-check the file by hand, and remove pairs
whose fix you would not accept in review. `eval.jsonl` holds 15% for testing
and must never be trained on.

## 5. Fine-tune in Google Colab

1. colab.research.google.com, **Runtime, Change runtime type, T4 GPU**.
2. Upload `train.jsonl` and `eval.jsonl` (Files panel, or from Google Drive).
3. Cells (the package APIs change; if a cell fails, check the current Unsloth docs):

```python
!pip install -q unsloth
from unsloth import FastLanguageModel
import json
from datasets import Dataset
from trl import SFTTrainer, SFTConfig

model, tok = FastLanguageModel.from_pretrained(
    "unsloth/Qwen2.5-Coder-7B-Instruct", max_seq_length=8192, load_in_4bit=True)
model = FastLanguageModel.get_peft_model(
    model, r=16, lora_alpha=16, lora_dropout=0,
    target_modules=["q_proj","k_proj","v_proj","o_proj","gate_proj","up_proj","down_proj"])

def load(path):
    rows = [json.loads(l) for l in open(path)]
    return Dataset.from_list([{"text": tok.apply_chat_template(r["messages"], tokenize=False)} for r in rows])

trainer = SFTTrainer(model=model, tokenizer=tok, train_dataset=load("train.jsonl"),
    eval_dataset=load("eval.jsonl"),
    args=SFTConfig(per_device_train_batch_size=1, gradient_accumulation_steps=8,
        num_train_epochs=2, learning_rate=2e-4, logging_steps=5, eval_strategy="epoch",
        output_dir="out", dataset_text_field="text", max_seq_length=8192, fp16=True))
trainer.train()

model.save_pretrained_gguf("misra-fixer", tok, quantization_method="q4_k_m")
```
4. Copy the `.gguf` file out of Colab (download it, or save it to Drive first).
   Sessions on the free tier end after a few hours: save to Drive often.

Files longer than the context length (8192 tokens above) do not fit: split big
files, or lower the length of the examples you keep.

## 6. Run the trained model on your PC

```bash
docker compose --profile llm up -d              # Ollama container, AMD GPU
docker/import-model.sh ~/Downloads/misra-fixer-q4_k_m.gguf misra-fixer
docker compose exec llm ollama run misra-fixer "int x = 010;"   # smoke test
```
Then repeat step 3 with `--provider-cmd /opt/misra/scripts/ai_providers/ollama_provider.py`.
GPU notes are in `docs/docker.md`.

## 7. Check that training helped

Run the converter with the base model and with the fine-tuned model over the
same files that were never in training (the eval files), and compare the share of
fixes that pass the gates. Keep the fine-tuned model only if that share is higher.
Then add the new verified pairs to the dataset and train again.

## Limits

- 12 GB of VRAM runs a 7B to 14B model comfortably; training larger ones needs a
  bigger GPU in Colab Pro or elsewhere.
- Hard rules (pointer casts, side effects) need many good examples.
- Fixes are suggestions. `--apply` stays off until a fix class has been validated.
- Do not put copied MISRA text in prompts or training data.
