#pragma once

namespace wuduo {

void set_nodelay(int fd, bool on = true);
void set_keep_alive(int fd, bool on = true);

int get_socket_error(int sockfd);
const char* strerror_thread_local(int saved_errno);

}
