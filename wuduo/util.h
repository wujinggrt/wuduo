#pragma once

namespace wuduo {

void set_nodelay(int fd, bool on = true);
void set_keep_alive(int fd, bool on = true);

}
