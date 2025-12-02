function get_target_for_redirect(target, domain) {
    const urlPattern = /^(https?:\/\/)?[a-zA-Z0-9.-]+(\.[a-zA-Z]{2,63})?(\/[a-zA-Z0-9.-]+)*\/?$/;
    if (urlPattern.test(target)) {
        if (target.includes(domain)) {
            return target;
        } else {
            return `https://${domain}`;
        }
    } else {
        return `https://${domain}`;
    }
}
