int strcmp(const char *lhs, const char *rhs) {
    while (*lhs && *rhs) {
        if (*lhs == *rhs) {
            lhs++;
            rhs++;
            continue;
        } else if (*lhs < *rhs) {
            return -1;
        } else if (*lhs > *rhs) {
            return 1;
        }
    }

    if (*lhs == 0 && *rhs == 0) {
        return 0;
    } else if (*lhs == 0) {
        return -1;
    } else {
        return 1;
    }
}