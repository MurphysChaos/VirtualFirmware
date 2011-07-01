
/*
 * anounce()
 * the announce function will multicast a message
 * announcing this functions service via a unique
 * magic number.
 *
 * Returns a socket
 */
int announce(char* d_port, int magic);

/*
 * locate()
 * the locate function will listen for incoming announcement messages
 * and will connect to the first valid server
 * 
 * Returns a socket
 */
int locate(int magic);
