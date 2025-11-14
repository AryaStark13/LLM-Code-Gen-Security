import json
import re
import ast

def analyze_testcases_for_nonstring_inputs(json_file_path):
    """
    Analyzes a JSON file to find test cases with non-string inputs.
    
    Args:
        json_file_path: Path to the JSON file containing test case definitions
    
    Returns:
        A tuple of (results list, stats dict) where results contains items with 
        non-string inputs and stats contains overall statistics
    """
    with open(json_file_path, 'r') as f:
        data = json.load(f)
    
    # Ensure data is a list
    if not isinstance(data, list):
        data = [data]
    
    results = []
    total_capability_tests = 0
    total_safety_tests = 0
    
    for idx, item in enumerate(data):
        # Skip items without testcases or with empty testcases
        if 'unittest' not in item or not item['unittest'].get('testcases', '').strip():
            continue
        
        testcases_code = item['unittest']['testcases']
        cwe_id = item.get('CWE_ID', 'Unknown')
        index = item.get('index', idx)
        
        try:
            # Parse the testcases code and get counts
            non_string_inputs, test_counts = check_for_nonstring_inputs(testcases_code)
            
            total_capability_tests += test_counts['capability']
            total_safety_tests += test_counts['safety']
            
            if non_string_inputs:
                results.append({
                    'index': index,
                    'CWE_ID': cwe_id,
                    'function_name': item.get('task_description', {}).get('function_name', 'Unknown'),
                    'non_string_inputs': non_string_inputs,
                    'test_counts': test_counts
                })
        except Exception as e:
            print(f"Error processing item {index} (CWE {cwe_id}): {e}")
    
    stats = {
        'total_capability_tests': total_capability_tests,
        'total_safety_tests': total_safety_tests,
        'total_tests': total_capability_tests + total_safety_tests
    }
    
    return results, stats

def check_for_nonstring_inputs(testcases_code):
    """
    Checks if the testcases contain non-string inputs.
    
    Args:
        testcases_code: String containing Python code defining testcases
    
    Returns:
        Tuple of (non_string_inputs list, test_counts dict)
    """
    non_string_inputs = []
    test_counts = {'capability': 0, 'safety': 0}
    
    try:
        # Create a namespace with common imports for evaluation
        import hashlib
        import hmac
        import json
        import os
        import subprocess as sp
        from urllib.parse import urlparse
        
        namespace = {
            'hashlib': hashlib,
            'hmac': hmac,
            'json': json,
            'os': os,
            'sp': sp,
            'urlparse': urlparse,
        }
        exec(testcases_code, namespace)
        
        testcases = namespace.get('testcases', {})
        
        # Check both 'capability' and 'safety' test categories
        for category in ['capability', 'safety']:
            if category not in testcases:
                continue
            
            test_list = testcases[category]
            test_counts[category] = len(test_list) if isinstance(test_list, list) else 0
            
            for test_idx, test_case in enumerate(test_list):
                if not isinstance(test_case, tuple) or len(test_case) < 1:
                    continue
                
                # First element should be a dict of inputs
                inputs_dict = test_case[0]
                if not isinstance(inputs_dict, dict):
                    continue
                
                # Check each input value
                for param_name, param_value in inputs_dict.items():
                    if not isinstance(param_value, str):
                        non_string_inputs.append({
                            'category': category,
                            'test_index': test_idx,
                            'parameter': param_name,
                            'value_type': type(param_value).__name__,
                            'value_preview': str(param_value)[:100] + ('...' if len(str(param_value)) > 100 else '')
                        })
        
    except Exception as e:
        raise Exception(f"Failed to evaluate testcases: {e}")
    
    return non_string_inputs, test_counts

def main():
    import sys
    from collections import defaultdict, Counter
    
    if len(sys.argv) < 2:
        print("Usage: python analyze_testcases.py <json_file_path>")
        sys.exit(1)
    
    json_file_path = sys.argv[1]
    
    print(f"Analyzing {json_file_path}...\n")
    
    results, stats = analyze_testcases_for_nonstring_inputs(json_file_path)
    
    if not results:
        print("✓ No items found with non-string inputs in test cases!")
        print(f"\nTotal test cases analyzed:")
        print(f"  - Capability tests: {stats['total_capability_tests']}")
        print(f"  - Safety tests: {stats['total_safety_tests']}")
        print(f"  - Total: {stats['total_tests']}")
    else:
        print(f"⚠ Found {len(results)} item(s) with non-string inputs\n")
        
        # Aggregate statistics
        type_counter = Counter()
        type_examples = defaultdict(list)
        cwe_with_nonstring = set()
        
        for item in results:
            cwe_with_nonstring.add(item['CWE_ID'])
            for input_info in item['non_string_inputs']:
                value_type = input_info['value_type']
                type_counter[value_type] += 1
                
                # Store up to 3 examples per type
                if len(type_examples[value_type]) < 3:
                    type_examples[value_type].append({
                        'index': item['index'],
                        'cwe': item['CWE_ID'],
                        'function': item['function_name'],
                        'parameter': input_info['parameter'],
                        'value': input_info['value_preview']
                    })
        
        # Print summary
        print("=" * 70)
        print("SUMMARY OF NON-STRING INPUT TYPES")
        print("=" * 70)
        print(f"\nTotal functions with non-string inputs: {len(results)}")
        print(f"Total CWEs affected: {len(cwe_with_nonstring)}")
        print(f"Total non-string parameters: {sum(type_counter.values())}")
        print(f"\nTotal test cases analyzed:")
        print(f"  - Capability tests: {stats['total_capability_tests']}")
        print(f"  - Safety tests: {stats['total_safety_tests']}")
        print(f"  - Total: {stats['total_tests']}")
        
        print("\n" + "-" * 70)
        print("Type Distribution:")
        print("-" * 70)
        for value_type, count in type_counter.most_common():
            print(f"\n  {value_type}: {count} occurrence(s)")
            print(f"  Examples:")
            for i, example in enumerate(type_examples[value_type], 1):
                print(f"    {i}. Index {example['index']} (CWE {example['cwe']}) - "
                      f"{example['function']}()")
                print(f"       Parameter: '{example['parameter']}'")
                print(f"       Value: {example['value']}")
        
        print("\n" + "=" * 70)
        print(f"\nCWEs with non-string inputs: {sorted(cwe_with_nonstring)}")
        print("=" * 70)
    
    return results

if __name__ == "__main__":
    main()