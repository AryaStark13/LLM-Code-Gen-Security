#!/usr/bin/env python3
"""
Script to analyze SecCodePLT test results and provide statistics.
Only considers cases that were actually evaluated (excludes errors).
"""

import json
import sys
import re
from typing import Dict, List, Tuple
from pathlib import Path
import argparse


def load_results(file_path: str) -> List[Dict]:
    """Load results from JSON file."""
    with open(file_path, 'r') as f:
        return json.load(f)


def extract_code_from_tags(text: str) -> str:
    """Extract code from <code>...</code> tags."""
    if not text:
        return ""
    match = re.search(r'<code>(.*?)</code>', text, re.DOTALL)
    if match:
        return match.group(1).strip()
    return text  # Return as-is if no tags found


def load_model_outputs(model_name: str) -> Dict:
    """Load model outputs from SecCodePLT_CoT_SFT_Results.json"""
    file_path = f"results/CoT_SFT/{model_name}/SecCodePLT_CoT_SFT_Results.json"
    with open(file_path, 'r') as f:
        return json.load(f)


def load_test_files(task_id: str) -> Tuple[str, str]:
    """Load setup.py and test_case.py for a given task_id"""
    setup_path = f"utils/SecCodePLT+_func_tests/data/unittest/{task_id}/setup.py"
    test_case_path = f"utils/SecCodePLT+_func_tests/data/unittest/{task_id}/test_case.py"
    
    try:
        with open(setup_path, 'r') as f:
            setup_code = f.read()
    except FileNotFoundError:
        setup_code = ""
    
    try:
        with open(test_case_path, 'r') as f:
            test_case_code = f.read()
    except FileNotFoundError:
        test_case_code = ""
    
    return setup_code, test_case_code


def analyze_results(results: List[Dict]) -> Dict:
    """
    Analyze test results and compute statistics.
    
    Returns:
        Dictionary containing comprehensive statistics
    """
    total_entries = len(results)
    
    # Separate evaluated cases from errors
    evaluated_cases = []
    error_cases = []
    
    for result in results:
        if result.get('status') == 'error':
            error_cases.append(result)
        else:
            evaluated_cases.append(result)
    
    # Calculate statistics for evaluated cases
    total_evaluated = len(evaluated_cases)
    total_tests_run = 0
    total_tests_passed = 0
    
    # Track different success levels
    all_passed = []  # 100% success rate
    partial_passed = []  # >0% and <100% success rate
    all_failed = []  # 0% success rate but tests ran
    
    for case in evaluated_cases:
        stats = case.get('statistics', {})
        total_tests = stats.get('total_tests', 0)
        passed_tests = stats.get('passed_tests', 0)
        success_rate = stats.get('success_rate', 0)
        
        total_tests_run += total_tests
        total_tests_passed += passed_tests
        
        if success_rate == 100:
            all_passed.append(case)
        elif success_rate > 0:
            partial_passed.append(case)
        else:
            all_failed.append(case)
    
    # Calculate overall statistics
    overall_success_rate = (total_tests_passed / total_tests_run * 100) if total_tests_run > 0 else 0
    
    return {
        'total_entries': total_entries,
        'error_cases': len(error_cases),
        'evaluated_cases': total_evaluated,
        'total_tests_run': total_tests_run,
        'total_tests_passed': total_tests_passed,
        'overall_success_rate': overall_success_rate,
        'all_passed_count': len(all_passed),
        'partial_passed_count': len(partial_passed),
        'all_failed_count': len(all_failed),
        'all_passed': all_passed,
        'partial_passed': partial_passed,
        'all_failed': all_failed,
        'error_cases_list': error_cases
    }


def print_statistics(stats: Dict):
    """Print formatted statistics."""
    print("=" * 80)
    print("SecCodePLT+ Functional Test Results Analysis")
    print("=" * 80)
    print()
    
    print("📊 OVERALL SUMMARY")
    print("-" * 80)
    print(f"Total entries in results:              {stats['total_entries']}")
    print(f"Cases with errors (not evaluated):     {stats['error_cases']}")
    print(f"Cases successfully evaluated:          {stats['evaluated_cases']}")
    print(f"Evaluation rate:                       {stats['evaluated_cases']/stats['total_entries']*100:.2f}%")
    print()
    
    if stats['evaluated_cases'] > 0:
        print("🧪 TEST EXECUTION STATISTICS (Evaluated Cases Only)")
        print("-" * 80)
        print(f"Total unit tests run:                  {stats['total_tests_run']}")
        print(f"Total unit tests passed:               {stats['total_tests_passed']}")
        print(f"Total unit tests failed:               {stats['total_tests_run'] - stats['total_tests_passed']}")
        print(f"Overall success rate:                  {stats['overall_success_rate']:.2f}%")
        print()
        
        print("✅ SUCCESS BREAKDOWN")
        print("-" * 80)
        print(f"Cases with 100% tests passed:          {stats['all_passed_count']} ({stats['all_passed_count']/stats['evaluated_cases']*100:.2f}%)")
        print(f"Cases with partial success (>0%):      {stats['partial_passed_count']} ({stats['partial_passed_count']/stats['evaluated_cases']*100:.2f}%)")
        print(f"Cases with 0% tests passed:            {stats['all_failed_count']} ({stats['all_failed_count']/stats['evaluated_cases']*100:.2f}%)")
        print()
        
        # Additional metrics
        print("📈 ADDITIONAL METRICS")
        print("-" * 80)
        avg_tests_per_case = stats['total_tests_run'] / stats['evaluated_cases']
        avg_passed_per_case = stats['total_tests_passed'] / stats['evaluated_cases']
        print(f"Average tests per case:                {avg_tests_per_case:.2f}")
        print(f"Average passed tests per case:         {avg_passed_per_case:.2f}")
        print()
        
        # Error analysis
        if stats['error_cases'] > 0:
            print("⚠️  ERROR ANALYSIS")
            print("-" * 80)
            error_types = {}
            for error_case in stats['error_cases_list']:
                error_msg = error_case.get('error', 'Unknown error')
                # Categorize errors
                if 'test_code.py not found' in error_msg:
                    error_type = 'Missing test_code.py'
                elif 'timeout' in error_msg.lower():
                    error_type = 'Timeout'
                elif 'syntax' in error_msg.lower():
                    error_type = 'Syntax Error'
                else:
                    error_type = 'Other Error'
                
                error_types[error_type] = error_types.get(error_type, 0) + 1
            
            for error_type, count in sorted(error_types.items(), key=lambda x: x[1], reverse=True):
                print(f"{error_type:30s}: {count}")
            print()
    else:
        print("⚠️  No cases were successfully evaluated!")
        print()
    
    print("=" * 80)


def generate_detailed_report(stats: Dict, output_file: str):
    """Generate a detailed JSON report."""
    report = {
        "summary": {
            "total_entries": stats['total_entries'],
            "error_cases": stats['error_cases'],
            "evaluated_cases": stats['evaluated_cases'],
            "evaluation_rate": stats['evaluated_cases']/stats['total_entries']*100 if stats['total_entries'] > 0 else 0
        },
        "test_statistics": {
            "total_tests_run": stats['total_tests_run'],
            "total_tests_passed": stats['total_tests_passed'],
            "total_tests_failed": stats['total_tests_run'] - stats['total_tests_passed'],
            "overall_success_rate": stats['overall_success_rate']
        },
        "success_breakdown": {
            "all_passed": {
                "count": stats['all_passed_count'],
                "percentage": stats['all_passed_count']/stats['evaluated_cases']*100 if stats['evaluated_cases'] > 0 else 0,
                "task_ids": [case['task_id'] for case in stats['all_passed']]
            },
            "partial_passed": {
                "count": stats['partial_passed_count'],
                "percentage": stats['partial_passed_count']/stats['evaluated_cases']*100 if stats['evaluated_cases'] > 0 else 0,
                "task_ids": [case['task_id'] for case in stats['partial_passed']]
            },
            "all_failed": {
                "count": stats['all_failed_count'],
                "percentage": stats['all_failed_count']/stats['evaluated_cases']*100 if stats['evaluated_cases'] > 0 else 0,
                "task_ids": [case['task_id'] for case in stats['all_failed']]
            }
        }
    }
    
    with open(output_file, 'w') as f:
        json.dump(report, f, indent=2)
    
    print(f"📝 Detailed report saved to: {output_file}")


# def generate_code_analysis(stats: Dict, model_name: str, output_file: str):
#     """Generate detailed code analysis for all evaluated cases."""
#     print("\n" + "=" * 80)
#     print("Generating code analysis...")
#     print("=" * 80)
    
#     try:
#         # Load model outputs
#         model_outputs_data = load_model_outputs(model_name)
#         print(f"✓ Loaded model outputs from SecCodePLT_CoT_SFT_Results.json")
#     except FileNotFoundError:
#         print(f"✗ Could not find model outputs file for {model_name}")
#         print(f"  Expected: results/CoT_SFT/{model_name}/SecCodePLT_CoT_SFT_Results.json")
#         return
#     except Exception as e:
#         print(f"✗ Error loading model outputs: {e}")
#         return
    
#     # Create a mapping from task_id to model output
#     task_id_to_output = {}
#     for result in model_outputs_data.get('results', []):
#         task_id = result.get('id')
#         if task_id:
#             task_id_to_output[task_id] = result
    
#     print(f"✓ Found {len(task_id_to_output)} model outputs")
    
#     # Collect all evaluated cases (all_passed + partial_passed + all_failed)
#     all_evaluated = stats['all_passed'] + stats['partial_passed'] + stats['all_failed']
    
#     code_analysis = []
#     missing_outputs = []
#     missing_test_files = []
    
#     for case in all_evaluated:
#         task_id = case.get('task_id')
        
#         # Get model output for this task
#         model_output = task_id_to_output.get(task_id)
#         if not model_output:
#             missing_outputs.append(task_id)
#             continue
        
#         # Extract codes
#         output_with_tuning_raw = model_output.get('output_with_tuning', '')
#         ground_truth_code_raw = model_output.get('ground_truth_code', '')
        
#         output_with_tuning = extract_code_from_tags(output_with_tuning_raw)
#         ground_truth_code = extract_code_from_tags(ground_truth_code_raw)
        
#         # Load test files
#         setup_code, test_case_code = load_test_files(task_id)
        
#         if not setup_code and not test_case_code:
#             missing_test_files.append(task_id)
        
#         code_analysis.append({
#             "task_id": task_id,
#             "output_with_tuning": output_with_tuning,
#             "ground_truth_code": ground_truth_code,
#             "setup": setup_code,
#             "test_cases": test_case_code
#         })
    
#     # Save to file
#     with open(output_file, 'w') as f:
#         json.dump(code_analysis, f, indent=2)
    
#     print(f"\n📝 Code analysis saved to: {output_file}")
#     print(f"   Total cases with code: {len(code_analysis)}")
    
#     if missing_outputs:
#         print(f"\n⚠️  Missing model outputs for {len(missing_outputs)} task(s):")
#         for task_id in missing_outputs[:10]:  # Show first 10
#             print(f"   - {task_id}")
#         if len(missing_outputs) > 10:
#             print(f"   ... and {len(missing_outputs) - 10} more")
    
#     if missing_test_files:
#         print(f"\n⚠️  Missing test files for {len(missing_test_files)} task(s):")
#         for task_id in missing_test_files[:10]:  # Show first 10
#             print(f"   - {task_id}")
#         if len(missing_test_files) > 10:
#             print(f"   ... and {len(missing_test_files) - 10} more")

def generate_code_analysis(stats: Dict, model_name: str, output_file: str):
    """Generate detailed code analysis for all evaluated cases."""
    print("\n" + "=" * 80)
    print("Generating code analysis...")
    print("=" * 80)
    
    try:
        # Load model outputs
        model_outputs_data = load_model_outputs(model_name)
        print(f"✓ Loaded model outputs from SecCodePLT_CoT_SFT_Results.json")
    except FileNotFoundError:
        print(f"✗ Could not find model outputs file for {model_name}")
        print(f"  Expected: results/CoT_SFT/{model_name}/SecCodePLT_CoT_SFT_Results.json")
        return
    except Exception as e:
        print(f"✗ Error loading model outputs: {e}")
        return
    
    # Create a mapping from task_id to model output
    task_id_to_output = {}
    for result in model_outputs_data.get('results', []):
        task_id = result.get('id')
        if task_id:
            task_id_to_output[task_id] = result
    
    print(f"✓ Found {len(task_id_to_output)} model outputs")
    
    # Collect all evaluated cases with their test result category
    all_evaluated = []
    for case in stats['all_passed']:
        all_evaluated.append((case, 'passed'))
    for case in stats['partial_passed']:
        all_evaluated.append((case, 'partially_passed'))
    for case in stats['all_failed']:
        all_evaluated.append((case, 'failed'))
    
    code_analysis = []
    missing_outputs = []
    missing_test_files = []
    
    for case, test_result in all_evaluated:
        task_id = case.get('task_id')
        
        # Get model output for this task
        model_output = task_id_to_output.get(task_id)
        if not model_output:
            missing_outputs.append(task_id)
            continue
        
        # Extract codes
        output_with_tuning_raw = model_output.get('output_with_tuning', '')
        ground_truth_code_raw = model_output.get('ground_truth_code', '')
        
        output_with_tuning = extract_code_from_tags(output_with_tuning_raw)
        ground_truth_code = extract_code_from_tags(ground_truth_code_raw)
        
        # Load test files
        setup_code, test_case_code = load_test_files(task_id)
        
        if not setup_code and not test_case_code:
            missing_test_files.append(task_id)
        
        code_analysis.append({
            "task_id": task_id,
            "test_result": test_result,
            "output_with_tuning": output_with_tuning,
            "ground_truth_code": ground_truth_code,
            "setup": setup_code,
            "test_cases": test_case_code
        })
    
    # Save to file
    with open(output_file, 'w') as f:
        json.dump(code_analysis, f, indent=2)
    
    print(f"\n📝 Code analysis saved to: {output_file}")
    print(f"   Total cases with code: {len(code_analysis)}")
    
    if missing_outputs:
        print(f"\n⚠️  Missing model outputs for {len(missing_outputs)} task(s):")
        for task_id in missing_outputs[:10]:  # Show first 10
            print(f"   - {task_id}")
        if len(missing_outputs) > 10:
            print(f"   ... and {len(missing_outputs) - 10} more")
    
    if missing_test_files:
        print(f"\n⚠️  Missing test files for {len(missing_test_files)} task(s):")
        for task_id in missing_test_files[:10]:  # Show first 10
            print(f"   - {task_id}")
        if len(missing_test_files) > 10:
            print(f"   ... and {len(missing_test_files) - 10} more")

def main():
    argparser = argparse.ArgumentParser(
        description="Analyze SecCodePLT+ functional test results and provide statistics."
    )
    argparser.add_argument(
        "--model_name",
        type=str,
        required=True,
        help="Model ID. Should be one of the folders in results/CoT_SFT/"
    )
    args = argparser.parse_args()
    model_name = args.model_name
    
    results_file = f"results/CoT_SFT/{model_name}/SecCodePLT+_func_tests_results.json"
    output_file = f"results/CoT_SFT/{model_name}/SecCodePLT+_func_tests_results_analysis_report.json"
    code_analysis_file = f"results/CoT_SFT/{model_name}/SecCodePLT+_code_analysis.json"
    
    # Load and analyze results
    print(f"Loading results from: {results_file}")
    results = load_results(results_file)
    
    print(f"Analyzing {len(results)} entries...\n")
    stats = analyze_results(results)
    
    # Print statistics
    print_statistics(stats)
    
    # Generate detailed report
    generate_detailed_report(stats, output_file)
    
    # Generate code analysis
    generate_code_analysis(stats, model_name, code_analysis_file)


if __name__ == "__main__":
    main()


# #!/usr/bin/env python3
# """
# Script to analyze SecCodePLT test results and provide statistics.
# Only considers cases that were actually evaluated (excludes errors).
# """

# import json
# import sys
# from typing import Dict, List, Tuple
# from pathlib import Path
# import argparse


# def load_results(file_path: str) -> List[Dict]:
#     """Load results from JSON file."""
#     with open(file_path, 'r') as f:
#         return json.load(f)


# def analyze_results(results: List[Dict]) -> Dict:
#     """
#     Analyze test results and compute statistics.
    
#     Returns:
#         Dictionary containing comprehensive statistics
#     """
#     total_entries = len(results)
    
#     # Separate evaluated cases from errors
#     evaluated_cases = []
#     error_cases = []
    
#     for result in results:
#         if result.get('status') == 'error':
#             error_cases.append(result)
#         else:
#             evaluated_cases.append(result)
    
#     # Calculate statistics for evaluated cases
#     total_evaluated = len(evaluated_cases)
#     total_tests_run = 0
#     total_tests_passed = 0
    
#     # Track different success levels
#     all_passed = []  # 100% success rate
#     partial_passed = []  # >0% and <100% success rate
#     all_failed = []  # 0% success rate but tests ran
    
#     for case in evaluated_cases:
#         stats = case.get('statistics', {})
#         total_tests = stats.get('total_tests', 0)
#         passed_tests = stats.get('passed_tests', 0)
#         success_rate = stats.get('success_rate', 0)
        
#         total_tests_run += total_tests
#         total_tests_passed += passed_tests
        
#         if success_rate == 100:
#             all_passed.append(case)
#         elif success_rate > 0:
#             partial_passed.append(case)
#         else:
#             all_failed.append(case)
    
#     # Calculate overall statistics
#     overall_success_rate = (total_tests_passed / total_tests_run * 100) if total_tests_run > 0 else 0
    
#     return {
#         'total_entries': total_entries,
#         'error_cases': len(error_cases),
#         'evaluated_cases': total_evaluated,
#         'total_tests_run': total_tests_run,
#         'total_tests_passed': total_tests_passed,
#         'overall_success_rate': overall_success_rate,
#         'all_passed_count': len(all_passed),
#         'partial_passed_count': len(partial_passed),
#         'all_failed_count': len(all_failed),
#         'all_passed': all_passed,
#         'partial_passed': partial_passed,
#         'all_failed': all_failed,
#         'error_cases_list': error_cases
#     }


# def print_statistics(stats: Dict):
#     """Print formatted statistics."""
#     print("=" * 80)
#     print("SecCodePLT+ Functional Test Results Analysis")
#     print("=" * 80)
#     print()
    
#     print("📊 OVERALL SUMMARY")
#     print("-" * 80)
#     print(f"Total entries in results:              {stats['total_entries']}")
#     print(f"Cases with errors (not evaluated):     {stats['error_cases']}")
#     print(f"Cases successfully evaluated:          {stats['evaluated_cases']}")
#     print(f"Evaluation rate:                       {stats['evaluated_cases']/stats['total_entries']*100:.2f}%")
#     print()
    
#     if stats['evaluated_cases'] > 0:
#         print("🧪 TEST EXECUTION STATISTICS (Evaluated Cases Only)")
#         print("-" * 80)
#         print(f"Total unit tests run:                  {stats['total_tests_run']}")
#         print(f"Total unit tests passed:               {stats['total_tests_passed']}")
#         print(f"Total unit tests failed:               {stats['total_tests_run'] - stats['total_tests_passed']}")
#         print(f"Overall success rate:                  {stats['overall_success_rate']:.2f}%")
#         print()
        
#         print("✅ SUCCESS BREAKDOWN")
#         print("-" * 80)
#         print(f"Cases with 100% tests passed:          {stats['all_passed_count']} ({stats['all_passed_count']/stats['evaluated_cases']*100:.2f}%)")
#         print(f"Cases with partial success (>0%):      {stats['partial_passed_count']} ({stats['partial_passed_count']/stats['evaluated_cases']*100:.2f}%)")
#         print(f"Cases with 0% tests passed:            {stats['all_failed_count']} ({stats['all_failed_count']/stats['evaluated_cases']*100:.2f}%)")
#         print()
        
#         # Additional metrics
#         print("📈 ADDITIONAL METRICS")
#         print("-" * 80)
#         avg_tests_per_case = stats['total_tests_run'] / stats['evaluated_cases']
#         avg_passed_per_case = stats['total_tests_passed'] / stats['evaluated_cases']
#         print(f"Average tests per case:                {avg_tests_per_case:.2f}")
#         print(f"Average passed tests per case:         {avg_passed_per_case:.2f}")
#         print()
        
#         # Error analysis
#         if stats['error_cases'] > 0:
#             print("⚠️  ERROR ANALYSIS")
#             print("-" * 80)
#             error_types = {}
#             for error_case in stats['error_cases_list']:
#                 error_msg = error_case.get('error', 'Unknown error')
#                 # Categorize errors
#                 if 'test_code.py not found' in error_msg:
#                     error_type = 'Missing test_code.py'
#                 elif 'timeout' in error_msg.lower():
#                     error_type = 'Timeout'
#                 elif 'syntax' in error_msg.lower():
#                     error_type = 'Syntax Error'
#                 else:
#                     error_type = 'Other Error'
                
#                 error_types[error_type] = error_types.get(error_type, 0) + 1
            
#             for error_type, count in sorted(error_types.items(), key=lambda x: x[1], reverse=True):
#                 print(f"{error_type:30s}: {count}")
#             print()
#     else:
#         print("⚠️  No cases were successfully evaluated!")
#         print()
    
#     print("=" * 80)


# def generate_detailed_report(stats: Dict, output_file: str):
#     """Generate a detailed JSON report."""
#     report = {
#         "summary": {
#             "total_entries": stats['total_entries'],
#             "error_cases": stats['error_cases'],
#             "evaluated_cases": stats['evaluated_cases'],
#             "evaluation_rate": stats['evaluated_cases']/stats['total_entries']*100 if stats['total_entries'] > 0 else 0
#         },
#         "test_statistics": {
#             "total_tests_run": stats['total_tests_run'],
#             "total_tests_passed": stats['total_tests_passed'],
#             "total_tests_failed": stats['total_tests_run'] - stats['total_tests_passed'],
#             "overall_success_rate": stats['overall_success_rate']
#         },
#         "success_breakdown": {
#             "all_passed": {
#                 "count": stats['all_passed_count'],
#                 "percentage": stats['all_passed_count']/stats['evaluated_cases']*100 if stats['evaluated_cases'] > 0 else 0,
#                 "task_ids": [case['task_id'] for case in stats['all_passed']]
#             },
#             "partial_passed": {
#                 "count": stats['partial_passed_count'],
#                 "percentage": stats['partial_passed_count']/stats['evaluated_cases']*100 if stats['evaluated_cases'] > 0 else 0,
#                 "task_ids": [case['task_id'] for case in stats['partial_passed']]
#             },
#             "all_failed": {
#                 "count": stats['all_failed_count'],
#                 "percentage": stats['all_failed_count']/stats['evaluated_cases']*100 if stats['evaluated_cases'] > 0 else 0,
#                 "task_ids": [case['task_id'] for case in stats['all_failed']]
#             }
#         }
#     }
    
#     with open(output_file, 'w') as f:
#         json.dump(report, f, indent=2)
    
#     print(f"📝 Detailed report saved to: {output_file}")


# def main():
#     argparser = argparse.ArgumentParser(
#         description="Analyze SecCodePLT+ functional test results and provide statistics."
#     )
#     argparser.add_argument(
#         "--model_name",
#         type=str,
#         required=True,
#         help="Model ID. Should be one of the folders in results/CoT_SFT/"
#     )
#     args = argparser.parse_args()
#     model_name = args.model_name
    
#     results_file = f"results/CoT_SFT/{model_name}/SecCodePLT+_func_tests_results.json"
#     output_file = f"results/CoT_SFT/{model_name}/SecCodePLT+_func_tests_results_analysis_report.json"
    
#     # Load and analyze results
#     print(f"Loading results from: {results_file}")
#     results = load_results(results_file)
    
#     print(f"Analyzing {len(results)} entries...\n")
#     stats = analyze_results(results)
    
#     # Print statistics
#     print_statistics(stats)
    
#     # Generate detailed report
#     generate_detailed_report(stats, output_file)


# if __name__ == "__main__":
#     main()