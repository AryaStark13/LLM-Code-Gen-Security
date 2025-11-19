# UNCHANGED utils/CWEval/cweval/generate.py
"""
Expected directory structure:

benchmark
├── core
│   ├── c
│   │   ├── cwe_022_0_c_task.c
│   └── py
│   |   ├── cwe_020_0_task.py
└── lang

evals
├── eval_241110_014704
│   ├── generated_0
│   │   ├── core
│   │   │   ├── c
│   │   │   │   ├── cwe_022_0_c_raw.c    <--- to generate
│   │   │   └── py
│   │   │       ├── cwe_020_0_raw.py
│   │   └── lang
│   └── generated_1
└── pytest.ini
"""

import datetime
import json
import os
import shutil
from typing import Any, Dict, List

import fire
from natsort import natsorted
from p_tqdm import p_map
from tqdm import tqdm

from cweval.ai import AIAPI
from cweval.commons import BENCHMARK_DIR, LANGS
from cweval.ppt import make_prompt


class Gener:

    begin_prompt_anchor = 'BEGIN PROMPT'
    begin_solution_anchor = 'BEGIN SOLUTION'

    def __init__(
        self,
        eval_path: str = '',
        model: str = 'gpt-4o-mini-2024-07-18',
        ppt: str = 'direct',
        num_proc: int = 8,
        langs: List[str] = LANGS,
        exclude_path: List[str] = [],
        include_path: List[str] = [],
        # AI parameters
        n: int = 20,
        max_completion_tokens: int = 2048,
        temperature: float = 0.8,
        **kwargs,
    ):
        self.model = model
        self.ppt = ppt
        self.num_proc = num_proc
        self.langs = langs
        self.exclude_path = exclude_path
        self.include_path = include_path
        print(f'Using langs: {self.langs}')
        self.ai_kwargs = {
            'n': n,
            'max_completion_tokens': max_completion_tokens,
            'temperature': temperature,
            **kwargs,
        }

        if not eval_path:
            self.eval_path = os.path.join(
                'evals', f'eval_{datetime.datetime.now().strftime("%y%m%d_%H%M%S")}'
            )
        else:
            # check if eval_path exists
            if os.path.exists(eval_path):
                flag = (
                    input(f'{eval_path} already exists, overwrite? (y/n): ')
                    .strip()
                    .lower()
                )
                if flag != 'y':
                    print(f'Exiting...')
                    exit(0)

            self.eval_path = eval_path

        self.cases = self._get_cases()

    def _get_cases(self) -> Dict[str, Dict[str, str]]:
        cases: Dict[str, str] = {}
        for root, _, files in os.walk(BENCHMARK_DIR):
            if '__pycache__' in root:
                continue
            for file in natsorted(files):
                file_wo_ext, ext = os.path.splitext(file)
                task_file_path = os.path.join(root, file)
                lang = ext[1:]
                # filtering
                if not (ext and file_wo_ext.endswith('_task')):
                    continue
                if lang not in self.langs:
                    continue
                if any(exclude in task_file_path for exclude in self.exclude_path):
                    continue
                if self.include_path and not any(
                    include in task_file_path for include in self.include_path
                ):
                    continue
                # gather code prompt
                with open(task_file_path, 'r') as f:
                    task_code = f.read()
                begin_solution_line_src = ''
                for line in task_code.splitlines():
                    if self.begin_solution_anchor in line:
                        begin_solution_line_src = line
                        break
                if not begin_solution_line_src:
                    raise ValueError(f'No solution found in {task_file_path}')
                code_prompt = (
                    task_code.split(self.begin_prompt_anchor)[-1]
                    .split(begin_solution_line_src)[0]
                    .strip()
                )

                rel_task_file_path = os.path.relpath(task_file_path, BENCHMARK_DIR)
                gen_file_path_template = os.path.join(
                    self.eval_path,
                    'generated_{index}',
                    rel_task_file_path.replace('_task', '_raw'),
                )

                cases[task_file_path] = {
                    'task_file_path': task_file_path,
                    'code_prompt': code_prompt,
                    'lang': lang,
                    'out_path_template': gen_file_path_template,
                }

        return cases

    @staticmethod
    def _gen_case(
        ai: str,
        ppt: str,
        case: Dict[str, str],
        ai_kwargs: Dict[str, Any],
        rank: int,
    ) -> None:
        num_samples = ai_kwargs.get('n', 1)
        for i in range(num_samples):
            out_path = case['out_path_template'].format(index=i)
            if not os.path.exists(out_path):
                break
        else:
            print(
                f'{case["out_path_template"]} already completed, skipping', flush=True
            )
            return

        aiapi = AIAPI(ai, **ai_kwargs)
        prompt = make_prompt(ppt)
        resps = prompt.req_ai(
            aiapi,
            case['lang'],
            case['code_prompt'],
            metadata={
                k: v for k, v in case.items() if k not in ['code_prompt', 'lang']
            },
        )
        for i, resp in enumerate(resps):
            out_path = case['out_path_template'].format(index=i)
            os.makedirs(os.path.dirname(out_path), exist_ok=True)
            with open(out_path, 'w') as f:
                f.write(resp)

    def gen(self) -> None:
        p_map(
            self._gen_case,
            [self.model] * len(self.cases),
            [self.ppt] * len(self.cases),
            self.cases.values(),
            [self.ai_kwargs] * len(self.cases),
            range(len(self.cases)),  # workaround: index as rank
            num_cpus=self.num_proc,
        )


if __name__ == "__main__":
    fire.Fire(Gener)

# # utils/CWEval/cweval/generate.py
# """
# Expected directory structure:

# benchmark
# ├── core
# │   ├── c
# │   │   ├── cwe_022_0_c_task.c
# │   └── py
# │   |   ├── cwe_020_0_task.py
# └── lang

# evals
# ├── eval_241110_014704
# │   ├── generated_0
# │   │   ├── core
# │   │   │   ├── c
# │   │   │   │   ├── cwe_022_0_c_raw.c    <--- to generate
# │   │   │   └── py
# │   │   │       ├── cwe_020_0_raw.py
# │   │   └── lang
# │   └── generated_1
# └── pytest.ini
# """

# import datetime
# import json
# import os
# import shutil
# from typing import Any, Dict, List

# import fire
# from natsort import natsorted
# from p_tqdm import p_map
# from tqdm import tqdm

# from cweval.ai import AIAPI
# from cweval.commons import BENCHMARK_DIR, LANGS
# from cweval.ppt import make_prompt
# from cweval.local_ai import LocalModelAPI


# class Gener:

#     begin_prompt_anchor = 'BEGIN PROMPT'
#     begin_solution_anchor = 'BEGIN SOLUTION'

#     def __init__(
#         self,
#         eval_path: str = '',
#         model: str = 'gpt-4o-mini-2024-07-18',
#         ppt: str = 'direct',
#         num_proc: int = 8,
#         langs: List[str] = LANGS,
#         exclude_path: List[str] = [],
#         include_path: List[str] = [],
#         # local model parameters
#         use_local_model: bool = False,
#         base_model_path: str = None,
#         sft_lora_adapter_path: str = None,
#         # AI parameters
#         n: int = 20, # of samples per task
#         max_completion_tokens: int = 2048,
#         temperature: float = 0.8,
#         **kwargs,
#     ):
#         self.model = model
#         self.ppt = ppt
#         self.num_proc = num_proc
#         self.langs = langs
#         self.exclude_path = exclude_path
#         self.include_path = include_path
#         # local model
#         self.use_local_model = use_local_model
#         self.base_model_path = base_model_path
#         self.sft_lora_adapter_path = sft_lora_adapter_path
#         print(f'Using langs: {self.langs}')
#         self.ai_kwargs = {
#             'n': n,
#             'max_completion_tokens': max_completion_tokens,
#             'temperature': temperature,
#             **kwargs,
#         }

#         if not eval_path:
#             self.eval_path = os.path.join(
#                 'evals', f'eval_{datetime.datetime.now().strftime("%y%m%d_%H%M%S")}'
#             )
#         else:
#             # check if eval_path exists
#             if os.path.exists(eval_path):
#                 flag = (
#                     input(f'{eval_path} already exists, overwrite? (y/n): ')
#                     .strip()
#                     .lower()
#                 )
#                 if flag != 'y':
#                     print(f'Exiting...')
#                     exit(0)

#             self.eval_path = eval_path

#         self.cases = self._get_cases()

#     def _get_cases(self) -> Dict[str, Dict[str, str]]:
#         cases: Dict[str, str] = {}
#         for root, _, files in os.walk(BENCHMARK_DIR):
#             if '__pycache__' in root:
#                 continue
#             for file in natsorted(files):
#                 file_wo_ext, ext = os.path.splitext(file)
#                 task_file_path = os.path.join(root, file)
#                 lang = ext[1:]
#                 # filtering
#                 if not (ext and file_wo_ext.endswith('_task')):
#                     continue
#                 if lang not in self.langs:
#                     continue
#                 if any(exclude in task_file_path for exclude in self.exclude_path):
#                     continue
#                 if self.include_path and not any(
#                     include in task_file_path for include in self.include_path
#                 ):
#                     continue
#                 # gather code prompt
#                 with open(task_file_path, 'r') as f:
#                     task_code = f.read()
#                 begin_solution_line_src = ''
#                 for line in task_code.splitlines():
#                     if self.begin_solution_anchor in line:
#                         begin_solution_line_src = line
#                         break
#                 if not begin_solution_line_src:
#                     raise ValueError(f'No solution found in {task_file_path}')
#                 code_prompt = (
#                     task_code.split(self.begin_prompt_anchor)[-1]
#                     .split(begin_solution_line_src)[0]
#                     .strip()
#                 )

#                 rel_task_file_path = os.path.relpath(task_file_path, BENCHMARK_DIR)
#                 gen_file_path_template = os.path.join(
#                     self.eval_path,
#                     'generated_{index}',
#                     rel_task_file_path.replace('_task', '_raw'),
#                 )

#                 cases[task_file_path] = {
#                     'task_file_path': task_file_path,
#                     'code_prompt': code_prompt,
#                     'lang': lang,
#                     'out_path_template': gen_file_path_template,
#                 }

#         return cases

#     # @staticmethod
#     # def _gen_case(
#     #     ai: str,
#     #     ppt: str,
#     #     case: Dict[str, str],
#     #     ai_kwargs: Dict[str, Any],
#     #     rank: int,
#     # ) -> None:
#     #     num_samples = ai_kwargs.get('n', 1)
#     #     for i in range(num_samples):
#     #         out_path = case['out_path_template'].format(index=i)
#     #         if not os.path.exists(out_path):
#     #             break
#     #     else:
#     #         print(
#     #             f'{case["out_path_template"]} already completed, skipping', flush=True
#     #         )
#     #         return

#     #     aiapi = AIAPI(ai, **ai_kwargs)
#     #     prompt = make_prompt(ppt)
#     #     resps = prompt.req_ai(
#     #         aiapi,
#     #         case['lang'],
#     #         case['code_prompt'],
#     #         metadata={
#     #             k: v for k, v in case.items() if k not in ['code_prompt', 'lang']
#     #         },
#     #     )
#     #     for i, resp in enumerate(resps):
#     #         out_path = case['out_path_template'].format(index=i)
#     #         os.makedirs(os.path.dirname(out_path), exist_ok=True)
#     #         with open(out_path, 'w') as f:
#     #             f.write(resp)

#     @staticmethod
#     def _gen_case_with_model(
#         aiapi,  # Pre-loaded model
#         ppt: str,
#         case: Dict[str, str],
#         ai_kwargs: Dict[str, Any],
#     ) -> None:
#         """Generate case using a pre-loaded model (for sequential processing)"""
#         from cweval.commons import extract_think_and_code
        
#         num_samples = ai_kwargs.get('n', 1)
        
#         # Check if all samples already exist
#         all_exist = True
#         for i in range(num_samples):
#             out_path = case['out_path_template'].format(index=i)
#             json_path = out_path.replace('_raw.', '_output.').rsplit('.', 1)[0] + '.json'
#             if not os.path.exists(json_path):
#                 all_exist = False
#                 break
        
#         if all_exist:
#             print(
#                 f'{case["out_path_template"]} already completed, skipping', flush=True
#             )
#             return

#         prompt = make_prompt(ppt)
#         resps = prompt.req_ai(
#             aiapi,
#             case['lang'],
#             case['code_prompt'],
#             metadata={
#                 k: v for k, v in case.items() if k not in ['code_prompt', 'lang']
#             },
#         )
        
#         for i, resp in enumerate(resps):
#             # Save raw output
#             out_path = case['out_path_template'].format(index=i)
#             os.makedirs(os.path.dirname(out_path), exist_ok=True)
#             with open(out_path, 'w') as f:
#                 f.write(resp)
            
#             # Extract think and code content
#             think_content, code_content = extract_think_and_code(resp)
            
#             # If extraction failed, try to use the raw response
#             if not code_content:
#                 print(f"WARNING: Could not extract <code> tags from {out_path}, using raw response")
#                 code_content = resp
            
#             # Create structured JSON output
#             json_output = {
#                 "task_id": case.get('task_file_path', '').replace('benchmark/', '').replace('_task.', '_'),
#                 "id": os.path.basename(case.get('task_file_path', '')).replace('_task.', '_').rsplit('.', 1)[0],
#                 "prompt": case['code_prompt'],
#                 "ground_truth_cot": "",
#                 "ground_truth_code": "",
#                 "ground_truth_full": "",
#                 "output_without_tuning": "",
#                 "think_output_with_tuning": think_content,
#                 "code_output_with_tuning": code_content,
#                 "full_output_with_tuning": resp,
#                 "has_extraction_warning": not code_content or not think_content,
#                 "generation_index": i,
#             }
            
#             # Save JSON
#             json_path = out_path.replace('_raw.', '_output.').rsplit('.', 1)[0] + '.json'
#             with open(json_path, 'w') as f:
#                 json.dump(json_output, f, indent=2)

#     @staticmethod
#     def _gen_case(
#         ai: str,
#         ppt: str,
#         case: Dict[str, str],
#         ai_kwargs: Dict[str, Any],
#         rank: int,
#         use_local_model: bool = False,
#         base_model_path: str = None,
#         sft_lora_adapter_path: str = None,
#     ) -> None:
#         from cweval.commons import extract_think_and_code
        
#         num_samples = ai_kwargs.get('n', 1)
        
#         # Check if all samples already exist
#         all_exist = True
#         for i in range(num_samples):
#             out_path = case['out_path_template'].format(index=i)
#             json_path = out_path.replace('_raw.', '_output.').rsplit('.', 1)[0] + '.json'
#             if not os.path.exists(json_path):
#                 all_exist = False
#                 break
        
#         if all_exist:
#             print(
#                 f'{case["out_path_template"]} already completed, skipping', flush=True
#             )
#             return

#         # Choose API based on use_local_model flag
#         if use_local_model:
#             aiapi = LocalModelAPI(
#                 base_model_path=base_model_path,
#                 sft_lora_adapter_path=sft_lora_adapter_path,
#                 **ai_kwargs
#             )
#         else:
#             aiapi = AIAPI(ai, **ai_kwargs)
        
#         prompt = make_prompt(ppt)
#         resps = prompt.req_ai(
#             aiapi,
#             case['lang'],
#             case['code_prompt'],
#             metadata={
#                 k: v for k, v in case.items() if k not in ['code_prompt', 'lang']
#             },
#         )
        
#         for i, resp in enumerate(resps):
#             # Save raw output
#             out_path = case['out_path_template'].format(index=i)
#             os.makedirs(os.path.dirname(out_path), exist_ok=True)
#             with open(out_path, 'w') as f:
#                 f.write(resp)
            
#             # Extract think and code content
#             think_content, code_content = extract_think_and_code(resp)
            
#             # If extraction failed, try to use the raw response
#             if not code_content:
#                 print(f"WARNING: Could not extract <code> tags from {out_path}, using raw response")
#                 code_content = resp
            
#             # Create structured JSON output
#             json_output = {
#                 "task_id": case.get('task_file_path', '').replace('benchmark/', '').replace('_task.', '_'),
#                 "id": os.path.basename(case.get('task_file_path', '')).replace('_task.', '_').rsplit('.', 1)[0],
#                 "prompt": case['code_prompt'],
#                 "ground_truth_cot": "",
#                 "ground_truth_code": "",
#                 "ground_truth_full": "",
#                 "output_without_tuning": "",
#                 "think_output_with_tuning": think_content,
#                 "code_output_with_tuning": code_content,
#                 "full_output_with_tuning": resp,
#                 "has_extraction_warning": not code_content or not think_content,
#                 "generation_index": i,
#             }
            
#             # Save JSON
#             json_path = out_path.replace('_raw.', '_output.').rsplit('.', 1)[0] + '.json'
#             with open(json_path, 'w') as f:
#                 json.dump(json_output, f, indent=2)

#     def gen(self) -> None:
#         # For local models, force single process to avoid CUDA multiprocessing issues
#         if self.use_local_model:
#             print("Using local model - running sequentially (num_proc=1) to avoid CUDA issues")
#             print(f"Generating code for {len(self.cases)} cases...")
            
#             # Load model once for all cases
#             aiapi = LocalModelAPI(
#                 base_model_path=self.base_model_path,
#                 sft_lora_adapter_path=self.sft_lora_adapter_path,
#                 **self.ai_kwargs
#             )
            
#             # Process cases sequentially
#             from tqdm import tqdm
#             for i, case in enumerate(tqdm(self.cases.values(), desc="Generating")):
#                 self._gen_case_with_model(
#                     aiapi=aiapi,
#                     ppt=self.ppt,
#                     case=case,
#                     ai_kwargs=self.ai_kwargs,
#                 )
#         else:
#             # Use multiprocessing for API-based models
#             p_map(
#                 self._gen_case,
#                 [self.model] * len(self.cases),
#                 [self.ppt] * len(self.cases),
#                 self.cases.values(),
#                 [self.ai_kwargs] * len(self.cases),
#                 range(len(self.cases)),
#                 [self.use_local_model] * len(self.cases),
#                 [self.base_model_path] * len(self.cases),
#                 [self.sft_lora_adapter_path] * len(self.cases),
#                 num_cpus=self.num_proc,
#             )


#     # def gen(self) -> None:
#     #     p_map(
#     #         self._gen_case,
#     #         [self.model] * len(self.cases),
#     #         [self.ppt] * len(self.cases),
#     #         self.cases.values(),
#     #         [self.ai_kwargs] * len(self.cases),
#     #         range(len(self.cases)),  # workaround: index as rank
#     #         num_cpus=self.num_proc,
#     #     )


# if __name__ == "__main__":
#     fire.Fire(Gener)
