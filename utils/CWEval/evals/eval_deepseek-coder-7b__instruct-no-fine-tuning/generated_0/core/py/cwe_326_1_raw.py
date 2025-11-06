def test_create_private_key_dsa():
    private_key = create_private_key_dsa()
    assert isinstance(private_key, str)
    assert len(private_key) > 0
    assert private_key.startswith("-----BEGIN PRIVATE KEY-----")
    assert private_key.endswith("-----END PRIVATE KEY-----")
