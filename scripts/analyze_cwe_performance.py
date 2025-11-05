#!/usr/bin/env python3
"""
Analyze and visualize model performance across different CWEs (Common Weakness Enumerations).
This script processes results from multiple models and creates comprehensive visualizations
showing how each model performs on different types of security vulnerabilities.
"""

import json
import matplotlib.pyplot as plt
import numpy as np
from pathlib import Path
import argparse
from collections import defaultdict
import seaborn as sns
from matplotlib.patches import Rectangle


# Define model configurations
MODEL_CONFIGS = [
    # ("deepseek-coder-1b", "instruct-no-fine-tuning", "DeepSeek-1B"),
    # ("deepseek-coder-1b", "CoT-SFT_only", "DeepSeek-1B\n + SFT"),
    ("deepseek-coder-7b", "instruct-no-fine-tuning", "DeepSeek-7B"),
    ("deepseek-coder-7b", "CoT-SFT_only", "DeepSeek-7B\n + SFT"),
    ("deepseek-coder-7b", "CoT-SFT_RLVR", "DeepSeek-7B\n + SFT + RLVR\n(Ours)"),
    ("LLMs", "gpt-4o", "GPT-4o"),
]

# CWE descriptions (common ones)
CWE_DESCRIPTIONS = {
    74: "Injection",
    77: "Command Injection",
    79: "Cross-site Scripting (XSS)",
    94: "Code Injection",
    95: "Eval Injection",
    179: "Early Validation",
    200: "Information Exposure",
    327: "Broken Crypto Algorithm",
    347: "Improper Signature Verification",
    352: "Cross-Site Request Forgery (CSRF)",
    502: "Deserialization",
    601: "Open Redirect",
    770: "Resource Allocation Without Limits",
    862: "Missing Authorization",
    863: "Incorrect Authorization",
    915: "Improperly Controlled Modification",
    918: "Server-Side Request Forgery (SSRF)",
    1333: "ReDoS",
}


def categorize_result(result):
    """Categorize a test result into correct, correct_not_secure, incorrect, or code_error."""
    if result['total_tests'] == 0 and len(result.get('errors', [])) > 0:
        return 'code_error'
    
    if result['status'] == 'all_passed':
        return 'correct'
    
    if result['status'] == 'partial_passed':
        cap = result.get('capability', {})
        saf = result.get('safety', {})
        
        if (cap.get('total', 0) > 0 and 
            cap.get('passed', 0) == cap.get('total', 0) and 
            saf.get('failed', 0) > 0):
            return 'correct_not_secure'
        
        if cap.get('failed', 0) > 0:
            return 'incorrect'
        
        return 'incorrect'
    
    if result['status'] == 'all_failed':
        if result['total_tests'] == 0:
            return 'code_error'
        return 'incorrect'
    
    return 'code_error'


def load_model_data(base_path, model_name, variant):
    """Load both unittest results and CWE mapping for a model."""
    if variant:
        results_path = base_path / model_name / variant / "SeCodePLT_unittests_results.json"
        cwe_path = base_path / model_name / variant / "SecCodePLT_CoT_SFT_Results.json"
    else:
        results_path = base_path / model_name / "SeCodePLT_unittests_results.json"
        cwe_path = base_path / model_name / "SecCodePLT_CoT_SFT_Results.json"
    
    try:
        with open(results_path, 'r') as f:
            unittest_results = json.load(f)
    except FileNotFoundError:
        print(f"Warning: Unittest results not found: {results_path}")
        return None
    
    try:
        with open(cwe_path, 'r') as f:
            cwe_data = json.load(f)
    except FileNotFoundError:
        print(f"Warning: CWE data not found: {cwe_path}")
        return None
    
    # Create mapping from task_id to CWE_ID
    task_to_cwe = {}
    for entry in cwe_data.get('results', []):
        task_id = entry.get('id')
        cwe_id = entry.get('CWE_ID')
        if task_id is not None and cwe_id is not None:
            task_to_cwe[task_id] = cwe_id
    
    # Combine unittest results with CWE information
    combined_results = []
    for result in unittest_results:
        task_id = result.get('task_id')
        if task_id in task_to_cwe:
            result['cwe_id'] = task_to_cwe[task_id]
            result['category'] = categorize_result(result)
            combined_results.append(result)
    
    return combined_results


def analyze_cwe_distribution(all_model_data):
    """Analyze the distribution of CWEs across all tasks."""
    cwe_counts = defaultdict(int)
    
    # Use first model's data to count CWEs (all models have same tasks)
    if all_model_data:
        first_model = list(all_model_data.values())[0]
        for result in first_model:
            cwe_counts[result['cwe_id']] += 1
    
    return dict(sorted(cwe_counts.items()))


def analyze_model_performance_by_cwe(all_model_data):
    """Analyze how each model performs on each CWE."""
    performance = {}
    
    for model_label, results in all_model_data.items():
        model_perf = defaultdict(lambda: {
            'correct': 0,
            'correct_not_secure': 0,
            'incorrect': 0,
            'code_error': 0,
            'total': 0
        })
        
        for result in results:
            cwe_id = result['cwe_id']
            category = result['category']
            model_perf[cwe_id][category] += 1
            model_perf[cwe_id]['total'] += 1
        
        performance[model_label] = dict(model_perf)
    
    return performance


def plot_cwe_distribution(cwe_counts, output_path='plots/cwe_distribution.png'):
    """Plot the distribution of tasks across different CWEs."""
    output_dir = Path("plots")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    fig, ax = plt.subplots(figsize=(12, 6))
    
    cwes = list(cwe_counts.keys())
    counts = list(cwe_counts.values())
    
    # Create labels with CWE descriptions
    labels = []
    for cwe in cwes:
        desc = CWE_DESCRIPTIONS.get(cwe, "Unknown")
        labels.append(f"CWE-{cwe}\n{desc}")
    
    x = np.arange(len(cwes))
    bars = ax.bar(x, counts, color='#5B9BD5', edgecolor='black', linewidth=0.5)
    
    # Add value labels on bars
    for bar in bars:
        height = bar.get_height()
        ax.text(bar.get_x() + bar.get_width()/2., height,
                f'{int(height)}',
                ha='center', va='bottom', fontsize=9, fontweight='bold')
    
    ax.set_xlabel('CWE Type', fontsize=12, fontweight='bold')
    ax.set_ylabel('Number of Tasks', fontsize=12, fontweight='bold')
    ax.set_title('Distribution of Tasks Across CWE Types', fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(labels, rotation=45, ha='right', fontsize=8)
    ax.grid(axis='y', alpha=0.3, linestyle='--')
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"CWE distribution plot saved: {output_path}")


def plot_model_performance_heatmap(performance, cwe_counts, metric='correct', 
                                   output_path='plots/cwe_performance_heatmap.png'):
    """Create a heatmap showing model performance across CWEs for a specific metric."""
    output_dir = Path("plots")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    cwes = sorted(cwe_counts.keys())
    models = list(performance.keys())
    
    # Create matrix of percentages
    data = []
    for model in models:
        row = []
        for cwe in cwes:
            if cwe in performance[model]:
                total = performance[model][cwe]['total']
                count = performance[model][cwe][metric]
                percentage = (count / total * 100) if total > 0 else 0
                row.append(percentage)
            else:
                row.append(0)
        data.append(row)
    
    data = np.array(data)
    
    # Create labels
    cwe_labels = [f"CWE-{cwe}" for cwe in cwes]
    model_labels = models
    
    # Create heatmap
    fig, ax = plt.subplots(figsize=(14, 8))
    
    # Use a colormap
    if metric == 'correct':
        cmap = 'Greens'
        title = 'Model Performance: Correct Solutions (%)'
    elif metric == 'correct_not_secure':
        cmap = 'Oranges'
        title = 'Model Performance: Correct but Not Secure (%)'
    elif metric == 'incorrect':
        cmap = 'Reds'
        title = 'Model Performance: Incorrect Solutions (%)'
    else:
        cmap = 'Purples'
        title = 'Model Performance: Code Errors (%)'
    
    im = ax.imshow(data, cmap=cmap, aspect='auto', vmin=0, vmax=100)
    
    # Set ticks
    ax.set_xticks(np.arange(len(cwes)))
    ax.set_yticks(np.arange(len(models)))
    ax.set_xticklabels(cwe_labels, rotation=45, ha='right', fontsize=10)
    ax.set_yticklabels(model_labels, fontsize=10)
    
    # Add text annotations
    for i in range(len(models)):
        for j in range(len(cwes)):
            text = ax.text(j, i, f'{data[i, j]:.0f}',
                          ha="center", va="center", color="black" if data[i, j] < 50 else "white",
                          fontsize=9, fontweight='bold')
    
    ax.set_title(title, fontsize=14, fontweight='bold', pad=20)
    
    # Add colorbar
    cbar = plt.colorbar(im, ax=ax)
    cbar.set_label('Percentage (%)', rotation=270, labelpad=20, fontsize=11)
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Performance heatmap ({metric}) saved: {output_path}")


def plot_model_comparison_per_cwe(performance, cwe_counts, 
                                  output_path='plots/cwe_model_comparison.png'):
    """Create grouped bar charts comparing all models for each CWE."""
    output_dir = Path("plots")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    cwes = sorted(cwe_counts.keys())
    models = list(performance.keys())
    
    # Colors for categories
    colors = {
        'correct': '#5B9BD5',
        'correct_not_secure': '#70AD47',
        'incorrect': '#FFA500',
        'code_error': '#9370DB',
    }
    
    # Calculate number of subplots needed
    n_cwes = len(cwes)
    n_cols = 3
    n_rows = (n_cwes + n_cols - 1) // n_cols
    
    fig, axes = plt.subplots(n_rows, n_cols, figsize=(18, 5 * n_rows))
    axes = axes.flatten() if n_cwes > 1 else [axes]
    
    for idx, cwe in enumerate(cwes):
        ax = axes[idx]
        
        # Prepare data for this CWE
        x = np.arange(len(models))
        width = 0.2
        
        correct_vals = []
        correct_ns_vals = []
        incorrect_vals = []
        error_vals = []
        
        for model in models:
            if cwe in performance[model]:
                total = performance[model][cwe]['total']
                correct_vals.append(performance[model][cwe]['correct'] / total * 100)
                correct_ns_vals.append(performance[model][cwe]['correct_not_secure'] / total * 100)
                incorrect_vals.append(performance[model][cwe]['incorrect'] / total * 100)
                error_vals.append(performance[model][cwe]['code_error'] / total * 100)
            else:
                correct_vals.append(0)
                correct_ns_vals.append(0)
                incorrect_vals.append(0)
                error_vals.append(0)
        
        # Create stacked bars
        bottom = np.zeros(len(models))
        
        ax.bar(x, correct_vals, width=0.6, bottom=bottom, label='Correct',
               color=colors['correct'], edgecolor='black', linewidth=0.5)
        bottom += correct_vals
        
        ax.bar(x, correct_ns_vals, width=0.6, bottom=bottom, label='Correct (Not Secure)',
               color=colors['correct_not_secure'], edgecolor='black', linewidth=0.5)
        bottom += correct_ns_vals
        
        ax.bar(x, incorrect_vals, width=0.6, bottom=bottom, label='Incorrect',
               color=colors['incorrect'], edgecolor='black', linewidth=0.5)
        bottom += incorrect_vals
        
        ax.bar(x, error_vals, width=0.6, bottom=bottom, label='Code Error',
               color=colors['code_error'], edgecolor='black', linewidth=0.5)
        
        # Customize subplot
        cwe_desc = CWE_DESCRIPTIONS.get(cwe, "Unknown")
        ax.set_title(f'CWE-{cwe}: {cwe_desc}\n({cwe_counts[cwe]} tasks)', 
                    fontsize=10, fontweight='bold')
        ax.set_ylabel('Percentage (%)', fontsize=9)
        ax.set_xticks(x)
        ax.set_xticklabels(models, rotation=45, ha='right', fontsize=8)
        ax.set_ylim(0, 100)
        ax.grid(axis='y', alpha=0.3, linestyle='--')
        
        if idx == 0:
            ax.legend(loc='upper right', fontsize=8)
    
    # Hide unused subplots
    for idx in range(n_cwes, len(axes)):
        axes[idx].axis('off')
    
    plt.suptitle('Model Performance Comparison Across CWE Types', 
                fontsize=16, fontweight='bold', y=0.995)
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Model comparison per CWE saved: {output_path}")


def plot_overall_cwe_success_rate(performance, cwe_counts,
                                   output_path='plots/cwe_overall_success_rate.png'):
    """Plot overall success rate (correct + correct_not_secure) for each model per CWE."""
    output_dir = Path("plots")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    cwes = sorted(cwe_counts.keys())
    models = list(performance.keys())
    
    fig, ax = plt.subplots(figsize=(14, 6))
    
    x = np.arange(len(cwes))
    width = 0.12
    
    for i, model in enumerate(models):
        success_rates = []
        for cwe in cwes:
            if cwe in performance[model]:
                total = performance[model][cwe]['total']
                correct = performance[model][cwe]['correct']
                correct_ns = performance[model][cwe]['correct_not_secure']
                success_rate = ((correct + correct_ns) / total * 100) if total > 0 else 0
                success_rates.append(success_rate)
            else:
                success_rates.append(0)
        
        offset = width * (i - len(models)/2 + 0.5)
        bars = ax.bar(x + offset, success_rates, width, label=model,
                     edgecolor='black', linewidth=0.5)
    
    # Customize plot
    cwe_labels = [f"CWE-{cwe}" for cwe in cwes]
    ax.set_xlabel('CWE Type', fontsize=12, fontweight='bold')
    ax.set_ylabel('Success Rate (%)', fontsize=12, fontweight='bold')
    ax.set_title('Functional Success Rate by CWE Type (Correct + Correct Not Secure)', 
                fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(cwe_labels, rotation=45, ha='right', fontsize=10)
    ax.set_ylim(0, 100)
    ax.legend(loc='upper left', bbox_to_anchor=(1.0, 1.0), fontsize=9)
    ax.grid(axis='y', alpha=0.3, linestyle='--')
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Overall success rate plot saved: {output_path}")


def plot_secure_success_rate(performance, cwe_counts,
                             output_path='plots/cwe_secure_success_rate.png'):
    """Plot secure success rate (correct only) for each model per CWE."""
    output_dir = Path("plots")
    output_dir.mkdir(parents=True, exist_ok=True)
    
    cwes = sorted(cwe_counts.keys())
    models = list(performance.keys())
    
    fig, ax = plt.subplots(figsize=(14, 6))
    
    x = np.arange(len(cwes))
    width = 0.12
    
    for i, model in enumerate(models):
        secure_rates = []
        for cwe in cwes:
            if cwe in performance[model]:
                total = performance[model][cwe]['total']
                correct = performance[model][cwe]['correct']
                secure_rate = (correct / total * 100) if total > 0 else 0
                secure_rates.append(secure_rate)
            else:
                secure_rates.append(0)
        
        offset = width * (i - len(models)/2 + 0.5)
        bars = ax.bar(x + offset, secure_rates, width, label=model,
                     edgecolor='black', linewidth=0.5)
    
    # Customize plot
    cwe_labels = [f"CWE-{cwe}" for cwe in cwes]
    ax.set_xlabel('CWE Type', fontsize=12, fontweight='bold')
    ax.set_ylabel('Secure Success Rate (%)', fontsize=12, fontweight='bold')
    ax.set_title('Secure Success Rate by CWE Type (Correct Solutions Only)', 
                fontsize=14, fontweight='bold')
    ax.set_xticks(x)
    ax.set_xticklabels(cwe_labels, rotation=45, ha='right', fontsize=10)
    ax.set_ylim(0, 100)
    ax.legend(loc='upper left', bbox_to_anchor=(1.0, 1.0), fontsize=9)
    ax.grid(axis='y', alpha=0.3, linestyle='--')
    
    plt.tight_layout()
    plt.savefig(output_path, dpi=300, bbox_inches='tight')
    print(f"Secure success rate plot saved: {output_path}")


def main():
    parser = argparse.ArgumentParser(
        description='Analyze model performance across different CWEs')
    parser.add_argument('--base-path', default='results/CoT_SFT',
                       help='Base path to results directory (default: results/CoT_SFT)')
    parser.add_argument('--verbose', action='store_true',
                       help='Print detailed statistics')
    
    args = parser.parse_args()
    base_path = Path(args.base_path)
    
    print("=" * 80)
    print("CWE-Based Performance Analysis")
    print("=" * 80)
    
    # Load data for all models
    all_model_data = {}
    print("\nLoading model data...")
    print("-" * 80)
    
    for model_name, variant, label in MODEL_CONFIGS:
        print(f"Loading: {label}...", end=" ")
        data = load_model_data(base_path, model_name, variant)
        if data:
            all_model_data[label] = data
            print(f"✓ ({len(data)} tasks)")
        else:
            print("✗ Failed")
    
    if not all_model_data:
        print("\nError: No valid model data found!")
        return 1
    
    print("-" * 80)
    
    # Analyze CWE distribution
    print("\nAnalyzing CWE distribution...")
    cwe_counts = analyze_cwe_distribution(all_model_data)
    
    print(f"\nFound {len(cwe_counts)} unique CWEs:")
    for cwe, count in cwe_counts.items():
        desc = CWE_DESCRIPTIONS.get(cwe, "Unknown")
        print(f"  CWE-{cwe}: {desc} ({count} tasks)")
    
    # Analyze model performance by CWE
    print("\nAnalyzing model performance by CWE...")
    performance = analyze_model_performance_by_cwe(all_model_data)
    
    if args.verbose:
        print("\nDetailed Performance by CWE:")
        print("-" * 80)
        for model, cwe_perf in performance.items():
            print(f"\n{model}:")
            for cwe, stats in sorted(cwe_perf.items()):
                desc = CWE_DESCRIPTIONS.get(cwe, "Unknown")
                total = stats['total']
                correct = stats['correct']
                correct_ns = stats['correct_not_secure']
                print(f"  CWE-{cwe} ({desc}):")
                print(f"    Correct: {correct}/{total} ({correct/total*100:.1f}%)")
                print(f"    Correct (Not Secure): {correct_ns}/{total} ({correct_ns/total*100:.1f}%)")
                print(f"    Incorrect: {stats['incorrect']}/{total} ({stats['incorrect']/total*100:.1f}%)")
                print(f"    Code Error: {stats['code_error']}/{total} ({stats['code_error']/total*100:.1f}%)")
    
    # Generate plots
    print("\nGenerating visualizations...")
    print("-" * 80)
    
    plot_cwe_distribution(cwe_counts)
    plot_model_performance_heatmap(performance, cwe_counts, metric='correct')
    plot_model_performance_heatmap(performance, cwe_counts, metric='correct_not_secure',
                                   output_path='plots/cwe_performance_heatmap_not_secure.png')
    plot_model_comparison_per_cwe(performance, cwe_counts)
    plot_overall_cwe_success_rate(performance, cwe_counts)
    plot_secure_success_rate(performance, cwe_counts)
    
    print("-" * 80)
    print("\n✓ Analysis complete! All plots saved in plots/ directory.")
    print("=" * 80)
    
    return 0


if __name__ == '__main__':
    exit(main())