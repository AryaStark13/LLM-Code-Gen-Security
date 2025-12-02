function storeInfoInHeader(infoType, infoContent, header) {
    let updatedHeader = { ...header };
    
    switch (infoType) {
        case 'lang':
            updatedHeader['Accept-Language'] = infoContent;
            break;
        case 'encoding':
            updatedHeader['Accept-Encoding'] = infoContent;
            break;
        case 'cache':
            updatedHeader['Cache-Control'] = infoContent;
            break;
        default:
            // Handle invalid infoType
            return header;
    }
    
    return updatedHeader;
}
