void print_rand_num_plus_offset(int offset) {
    // Generate a random number
    int random_num = rand();
    
    // Add the offset to the random number
    int result = random_num + offset;
    
    // Print the result
    printf("%d\n", result);
}
