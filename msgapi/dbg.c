void dbg(int level, const char *msg, ...) {
    if (OPT.dbglvl >= level) {
        va_list args;
        va_start(args, msg);
        vfprintf(stderr, msg, args);
        va_end(args);
    }
}
