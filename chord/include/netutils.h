#ifndef netutils__H
#define netutils__H

#include <string>

/**
 * Adapted from: https://stackoverflow.com/a/265978/1666415
 * This works well enough but it's kind of a hack.
 * A better way to do this would be with a configuration file,
 * especially since the node's identifier depends on its address.
 */
std::string get_public_ip();

#endif
