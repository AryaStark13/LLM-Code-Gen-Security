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


// BEGIN ENTRYPOINT
if (require.main === module) {
    const args = process.argv.slice(2);
    const target = args[0];
    const domain = args[1];
    if (!target || !domain) {
        console.error("Please provide both the target URL and domain.");
        process.exit(1);
    }
    console.log(get_target_for_redirect(target, domain));
}
