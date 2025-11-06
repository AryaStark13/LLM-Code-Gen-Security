def test_create_private_key():
    private_key = create_private_key()
    assert isinstance(private_key, str)
    assert len(private_key) > 0
    print("All test cases pass")

test_create_private_key()
