#include "server.h"



int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_fd, new_socket;
    int opt = 1;
    

    /* Create a TCP socket */
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    // corner case
    if (server_fd < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    /* get the server address information */
    struct sockaddr_in server_addr = {0}; // sth handle internet address; a struct
    int addrlen = sizeof(server_addr);
    server_addr.sin_family = AF_INET; // set address family; specify using IPv4
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); // specify the IP addr for socket; htonl is to ensure correct byte order for network (host to network long)
    server_addr.sin_port = htons(port); // host to network short


    /* Binding the socket to the server address*/ 
    // associates socket with specific IP and port#
    int bind_result = bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr));

    //If bind fails, try different port by user
    if(bind_result < 0){
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Listening on port %d\n", port);

    // Accept clients in a loop
    while (1) {
        new_socket = accept(server_fd, (struct sockaddr*)&server_addr, (socklen_t*)&addrlen);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        Client* new_client = malloc(sizeof(Client));
        new_client->sockfd = new_socket;
        new_client->address = server_addr;
        new_client->addr_len = addrlen;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, new_client) != 0) {
            perror("could not create thread");
            continue;
        }

        // Detach the thread so it cleans up after itself
        pthread_detach(thread_id);
    }

    // Cleanup before shutdown (will not reach here in the current form)
    close(server_fd);

    return 0;
}
