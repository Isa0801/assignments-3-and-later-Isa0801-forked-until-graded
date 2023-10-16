#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <syslog.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdbool.h>
#include <sys/queue.h>
#include <pthread.h> 

#define LOGGING_CLIENT_DATA_PATH "/var/tmp/aesdsocketdata"

struct entry_thread
{
	pthread_t thread_fd;
	SLIST_ENTRY(entry_thread) entries;
};

SLIST_HEAD(link_head, entry_thread) list_head;

int socket_fd = 0, client_fd = 0;
const int max_connection = 10;
struct sockaddr_in addr;
int addr_size = sizeof(struct sockaddr_in);
char client_addr[INET_ADDRSTRLEN];
bool daemon_b = false;
bool running_timestamp = true;
pthread_mutex_t global_mutex;

int read_from_aesdsockdata(char *data, int *buffer_size)
{
	int fd = open(LOGGING_CLIENT_DATA_PATH, O_RDWR|O_APPEND|O_CREAT, S_IRWXO|S_IRWXG|S_IRWXU);
	
	if (fd == -1)
	{
		exit(-1);
	}
	
	int counter = 0;
	int res = -1;
	
	
	while((res = read(fd, &data[counter], 1)) == 1)
    {
        if (res == -1)
        {
            exit(-1);
        }

        if (data[counter] == '\0')
        {
            break;
        }

        counter++;

        if ((counter) >= *buffer_size)
        {
            *buffer_size += 512;
            data = realloc(data, *buffer_size);
        }
    }
	
	close(fd);
	
	return counter;
	
}

void write_to_aesdsockdata(char *data_buffer, const int len)
{
	int fd = open(LOGGING_CLIENT_DATA_PATH, O_RDWR|O_APPEND|O_CREAT, S_IRWXO|S_IRWXG|S_IRWXU);
	
	if (fd == -1)
	{
		exit(-1);
	}
	
	write(fd, data_buffer, len);
	
	close(fd);
}

void before_exit()
{

	running_timestamp = false;

	while (!SLIST_EMPTY(&list_head)) {
		printf("removing threads:\n");
		struct entry_thread *p_thread = SLIST_FIRST(&list_head);
		pthread_join((p_thread->thread_fd), NULL);
		SLIST_REMOVE_HEAD(&list_head, entries);
		free(p_thread);
	}

	if (client_fd != -1)
	{
			close(client_fd);
	}

	if (socket_fd == -1 || socket_fd == 0)
	{
			shutdown(socket_fd, SHUT_RDWR); 
	}

	closelog();
	
	printf("done\n");
	remove(LOGGING_CLIENT_DATA_PATH);
	
}

void handel_signal()
{
    syslog(LOG_USER, "Caught signal, exiting");
    exit(0);
}

void* handle_connection_threaded(void*);

void handle_timerstamp_signal();


int main(int argc, char **argv)
{
	
	for (int i = 0; i < argc; i++)
	{
		
		if (strcmp(argv[i], "-d") == 0)
		{
			daemon_b = true;
		}
	}

	SLIST_INIT(&list_head);

	openlog("aesdsocket", 0, LOG_USER);
	
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	addr = (struct sockaddr_in){.sin_family = AF_INET, .sin_port = htons(9000), .sin_addr = (struct in_addr){.s_addr = INADDR_ANY}};

	signal(SIGTERM, handel_signal);
	signal(SIGINT, handel_signal);
	signal(SIGALRM, handle_timerstamp_signal);
	
	int option = 1;
	
	if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option)) == -1)
	{
		exit(-1);
	}
	
	if (atexit(&before_exit))
	{
		exit(-1);
	}

    if (socket_fd == -1)
    {
        exit(-1);
    }


    if (bind(socket_fd, (struct sockaddr *) &addr, sizeof(struct sockaddr_in)))
    {
        exit(-1);
    }

    if (listen(socket_fd, max_connection) == -1)
    {
        exit(-1);
    }
	
	if (daemon_b)
	{
		pid_t pid = fork();

		if (pid != 0)
		{
			exit(0);
		}
	}


	alarm(10);


	while (1)
	{
		printf("waiting for new connection\n");
		client_fd = accept(socket_fd, (struct sockaddr *) &addr, &addr_size);
		printf("accepted %d\n", client_fd);

		if (client_fd == -1)
		{
			exit(-1);
		}

		struct entry_thread *p_thread  = malloc(sizeof(struct entry_thread));

		inet_ntop(AF_INET, &addr.sin_addr, client_addr, INET_ADDRSTRLEN);
		syslog(LOG_USER, "Accepted connection from %s", client_addr);

		int *thread_client_fd = malloc(sizeof(int));
		*thread_client_fd = client_fd;

		pthread_create(&p_thread->thread_fd, NULL, &handle_connection_threaded, thread_client_fd);

		if (SLIST_EMPTY(&list_head))
		{
			SLIST_INSERT_HEAD(&list_head, p_thread, entries);
		} else if (!SLIST_EMPTY(&list_head))
		{
			SLIST_INSERT_AFTER(SLIST_FIRST(&list_head), p_thread, entries);
		} else
		{
			printf("could not add\n");
		}

		client_fd = -1;
		
	}

	return -1;

}


void handle_timerstamp_signal()
{

	time_t current_time;
	struct tm *tm_time;
	char buffer[50];


	time(&current_time);

	tm_time = localtime(&current_time);
	int bytes = strftime(buffer, 50, "timestamp:%c\n", tm_time);

	pthread_mutex_lock(&global_mutex);
	write_to_aesdsockdata(buffer, bytes);
	pthread_mutex_unlock(&global_mutex);

	alarm(10);

}

void* handle_connection_threaded(void *arg)
{
	int counter = 0;
	int res;
	int client_fd = *(int*)arg;
	free(arg);
	int data_buffer_size = sizeof(char) * 1024;
	char *data_buffer = malloc(sizeof(char) * data_buffer_size);

	printf("starting to read from %d\n", client_fd);
	while((res = recv(client_fd, &data_buffer[counter], 1, 0)) == 1)
	{
		printf("read: %c\n", data_buffer[counter]);
		if (res == -1)
		{
			printf("err%d\n", client_fd);
			pthread_exit(NULL);
		}

		if (data_buffer[counter] == '\n' || data_buffer[counter] == '\0')
		{
			break;
		}

		counter++;

		if ((counter) >= data_buffer_size)
		{
			data_buffer_size += 512;
			data_buffer = realloc(data_buffer, data_buffer_size);
		}
	}

	if ((counter + 2) >= data_buffer_size)
	{
		data_buffer_size += 512;
		data_buffer = realloc(data_buffer, data_buffer_size);
	}

	char *newline = strchr(data_buffer, '\n');

	*(newline + 1) = '\0';

	pthread_mutex_lock(&global_mutex);
	printf("read%s\n", data_buffer);
	write_to_aesdsockdata(data_buffer, (newline - data_buffer) + 1);
	pthread_mutex_unlock(&global_mutex);




	pthread_mutex_lock(&global_mutex);
	int data_to_send = read_from_aesdsockdata(data_buffer, &data_buffer_size);
	pthread_mutex_unlock(&global_mutex);
	
	pthread_mutex_lock(&global_mutex);
	write(client_fd, data_buffer, data_to_send);
	pthread_mutex_unlock(&global_mutex);

	printf("closing off%d\n", client_fd);

	if (client_fd != -1)
		close(client_fd);


	free(data_buffer);
}