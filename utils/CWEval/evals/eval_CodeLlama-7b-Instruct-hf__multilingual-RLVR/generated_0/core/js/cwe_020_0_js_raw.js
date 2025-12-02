function get_target_for_redirect(target, domain) {
    return target.startsWith('https://' + domain) ? target : 'https://' + domain;
}
