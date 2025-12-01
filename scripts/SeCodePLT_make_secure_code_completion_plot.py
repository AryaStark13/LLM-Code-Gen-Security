#!/usr/bin/env python3
"""
Generate a stacked bar chart showing secure code completion results for Python.
Processes results from multiple model configurations and visualizes the distribution
of correct, correct (not secure), incorrect, and error categories.
"""

import json
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import argparse


# Define model configurations: (model_folder, variant_folder, display_label)
MODEL_CONFIGS = [
    # ("deepseek-coder-1b", "instruct-no-fine-tuning", "DeepSeek-1B"),
    # ("deepseek-coder-1b", "CoT-SFT_only", "DeepSeek-1B\n + SFT"),
    ("deepseek-coder-7b", "instruct-no-fine-tuning", "DeepSeek-7B"),
    ("deepseek-coder-7b", "CoT-SFT_only", "DeepSeek-7B\n + SFT"),
    ("deepseek-coder-7b", "CoT-SFT_RLVR", "DeepSeek-7B\n + SFT + RLVR\n(Ours)"),
    ("LLMs", "gpt-4o", "GPT-4o"),
    # ("LLMs", "gpt-5", "GPT-5"),
    # ("LLMs", "gpt-5-mini", "GPT-5-Mini"),
]

# MODEL_CONFIGS = [
#     # ("deepseek-coder-1b", "instruct-no-fine-tuning", "DeepSeek-1B"),
#     # ("deepseek-coder-1b", "CoT-SFT_only", "DeepSeek-1B\n + SFT"),
#     ("deepseek-coder-7b", "instruct-no-fine-tuning", "DeepSeek-7B"),
#     ("deepseek-coder-7b", "CoT-SFT_only", "DeepSeek-7B\n + SFT"),
#     ("deepseek-coder-7b", "CoT-SFT_RLVR", "DeepSeek-7B\n + SFT + RLVR\n(Ours)"),
#     ("LLMs", "gpt-4o", "GPT-4o"),
# ]

# Color scheme matching the reference plot
COLORS = {
    'correct': '#5B9BD5',  # blue
    'correct_not_secure': '#70AD47',  # green
    'incorrect': '#FFA500',  # orange
    # 'syntax_error': '#9370DB',  # purple
    # 'other_error': '#A9A9A9',  # gray
    'code_error': '#9370DB',  # tomato red
}

# Display labels for legend
LABELS = {
    'correct': 'correct',
    'correct_not_secure': 'correct (not secure)',
    'incorrect': 'incorrect',
    # 'syntax_error': 'syntax error',
    # 'other_error': 'other error',
    'code_error': 'code error',
}


def categorize_result(result):
    """
    Categorize a single test result based on its status and error information.
    
    Categories:
    - correct: All tests (capability + safety) passed
    - correct_not_secure: Capability tests passed but safety tests failed
    - incorrect: Capability tests failed (functional errors)
    - code_error: Code execution errors (syntax or runtime)
    (deprecated)
    - syntax_error: Syntax errors (0 tests run)
    - other_error: Other types of errors
    """
    # Check for syntax errors (0 tests run with errors)
    if result['total_tests'] == 0 and len(result.get('errors', [])) > 0:
        # Check if it's a syntax error
        for error in result['errors']:
            # error_msg = error.get('error_message', '')
            # error_reason = error.get('reason', '')
            # if 'SyntaxError' in error_msg or 'invalid syntax' in error_msg:
            #     return 'syntax_error'
            # print(f"Debug: Other error message found: {error_msg}")
            # print(f"Debug: Error reason: {error_reason}")
            # return 'other_error'
            return 'code_error'
    
    # Label to match with SeCodePLT: "correct" 
    # All tests passed (both capability and safety)
    if result['status'] == 'all_passed':
        return 'correct'
    
    # Label to match with SeCodePLT: "correct_not_secure"
    # Partial pass - differentiate between capability and safety failures
    if result['status'] == 'partial_passed':
        cap = result.get('capability', {})
        saf = result.get('safety', {})
        
        # All capability tests passed but at least one safety test failed
        if (cap.get('total', 0) > 0 and 
            cap.get('passed', 0) == cap.get('total', 0) and 
            saf.get('failed', 0) > 0):
            return 'correct_not_secure'
        
        # Capability tests failed (functional incorrectness)
        if cap.get('failed', 0) > 0:
            return 'incorrect'
        
        # Default to incorrect for other partial pass cases
        return 'incorrect'
    
    # Label to match with SeCodePLT: "incorrect"
    # All tests failed
    if result['status'] == 'all_failed':
        # Check if it's due to syntax error (0 tests run)
        if result['total_tests'] == 0:
            # return 'syntax_error'
            return 'code_error'
        return 'incorrect'
    
    # Fallback for unexpected cases
    # return 'other_error'
    return 'code_error'


def process_results(json_path):
    """
    Process a results JSON file and return category statistics.
    
    Returns:
        tuple: (percentages dict, counts dict, total number of tests)
    """
    try:
        with open(json_path, 'r') as f:
            results = json.load(f)
    except FileNotFoundError:
        print(f"Warning: File not found: {json_path}")
        return None, None, 0
    except json.JSONDecodeError:
        print(f"Warning: Invalid JSON in file: {json_path}")
        return None, None, 0
    
    # Initialize counts
    counts = {
        'correct': 0,
        'correct_not_secure': 0,
        'incorrect': 0,
        # 'syntax_error': 0,
        # 'other_error': 0,
        'code_error': 0,
    }
    
    # Categorize each result
    for result in results:
        category = categorize_result(result)
        counts[category] += 1
    
    # Calculate percentages
    # total = 25  # Fixed total number of tasks
    # total = 386  # Fixed total number of tasks
    total = len(results)
    percentages = {
        cat: (count / total * 100) if total > 0 else 0 
        for cat, count in counts.items()
    }

    # set remaining to other_error: total - len(results)
    remaining = total - len(results)
    if remaining > 0:
        counts['code_error'] += remaining
        percentages['code_error'] = (counts['code_error'] / total * 100)
    
    return percentages, counts, total


def create_plot(data, output_path='secure_code_completion_plot.png'):
    """
    Create a stacked bar chart showing results for all models.
    
    Args:
        data: List of dicts containing label, percentages, counts, and total
        output_path: Path to save the output plot
    """
    output_dir = Path("plots")
    output_dir.mkdir(parents=True, exist_ok=True)
    output_path = output_dir / output_path
    fig, ax = plt.subplots(figsize=(14, 5))
    
    # Prepare data for stacking
    labels = [d['label'] for d in data]
    x = np.arange(len(labels))
    width = 0.8
    
    # Initialize bottom positions for stacking
    bottom = np.zeros(len(labels))
    
    # Stack bars in order
    # category_order = ['correct', 'correct_not_secure', 'incorrect', 
    #                   'syntax_error', 'other_error']
    category_order = ['correct', 'correct_not_secure', 'incorrect', 
                      'code_error']
    
    for cat in category_order:
        values = [d['percentages'][cat] for d in data]
        bars = ax.bar(x, values, width, bottom=bottom,
                     label=LABELS[cat],
                     color=COLORS[cat],
                     edgecolor='black',
                     linewidth=0.5)
        
        # Add percentage labels on bars (only if segment is large enough)
        for i, (bar, val) in enumerate(zip(bars, values)):
            if val > 3:  # Only show label if segment is visible
                height = bar.get_height()
                count = data[i]['counts'][cat]
                # Show percentage value
                ax.text(bar.get_x() + bar.get_width()/2., 
                       bottom[i] + height/2.,
                       f'{int(val)}',
                    #    f'{val:.1f}',
                    #    f'{round(val)}',
                       ha='center', va='center', 
                       fontsize=16, fontweight='bold',
                    #    color='white' if cat == 'syntax_error' else 'black')
                       color='white' if cat == 'code_error' else 'black')
        
        bottom += values
    
    # Customize plot appearance
    ax.set_ylabel('Percentage of Test Cases', fontsize=16)
    ax.set_title('SeCodePLT Results on the Test Split (85 Examples)', fontsize=22, fontweight='bold')
    # ax.set_title('CWEval Results on the Python Split (85 Examples)', fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    # ax.set_xticklabels(labels, rotation=45, ha='right', fontsize=10)
    ax.set_xticklabels(labels, fontsize=16)
    # Make the RLVR label bold
    tick_labels = ax.get_xticklabels()
    for tick_label in tick_labels:
        if 'RLVR' in tick_label.get_text():
            tick_label.set_weight('bold')
            # tick_label.set_color('#FF4500')  # Optional: change color to highlight
            # change actual background color of label like a highlight
            tick_label.set_bbox(dict(facecolor='#FFFFE0', edgecolor='none', pad=2.0))
    ax.set_ylim(0, 100)
    # ax.legend(loc='upper left', bbox_to_anchor=(1.02, 1), frameon=True)
    ax.legend(loc='upper left', bbox_to_anchor=(1.02, 1), frameon=True, fontsize=16, 
              title='Result Categories', title_fontsize=16,
              fancybox=True)
    ax.grid(axis='y', alpha=0.3, linestyle='--')
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"\nPlot saved as '{output_path}'")
    # plt.show()


def main():
    parser = argparse.ArgumentParser(
        description='Generate secure code completion results plot')
    parser.add_argument('--base-path', default='results/CoT_SFT',
                       help='Base path to results directory (default: results/CoT_SFT)')
    parser.add_argument('--output', default='secure_code_completion_plot.png',
                       help='Output plot filename (default: secure_code_completion_plot.png)')
    parser.add_argument('--verbose', action='store_true',
                       help='Print detailed statistics')
    
    args = parser.parse_args()
    base_path = Path(args.base_path)
    
    # Collect data for all models
    data = []
    print("Processing results...")
    print("-" * 60)
    
    for model_name, variant, label in MODEL_CONFIGS:
        # Construct path to results file
        if variant:
            json_path = base_path / model_name / variant / "SeCodePLT_unittests_results.json"
            # json_path = base_path / model_name / variant / "CWEval_unittests_results.json"
        else:
            json_path = base_path / model_name / "SeCodePLT_unittests_results.json"
            # json_path = base_path / model_name / "CWEval_unittests_results.json"
        
        percentages, counts, total = process_results(json_path)
        
        if percentages is not None:
            data.append({
                'label': label,
                'percentages': percentages,
                'counts': counts,
                'total': total
            })
            
            if args.verbose:
                print(f"\n{label}:")
                print(f"  Total tasks: {total}")
                # for cat in ['correct', 'correct_not_secure', 'incorrect', 
                #            'syntax_error', 'other_error']:
                for cat in ['correct', 'correct_not_secure', 'incorrect', 
                           'code_error']:
                    print(f"  {LABELS[cat]}: {counts[cat]} ({percentages[cat]:.1f}%)")
            else:
                print(f"{label}: {total} tasks processed")
    
    print("-" * 60)
    
    if not data:
        print("Error: No valid result files found!")
        return 1
    
    # Create the plot
    create_plot(data, args.output)
    
    return 0


if __name__ == '__main__':
    exit(main())