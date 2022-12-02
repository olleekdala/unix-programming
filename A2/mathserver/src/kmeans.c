#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <limits.h>

#define MAX_POINTS 4096 * 4096
#define MAX_CLUSTERS 32 * 32

typedef struct point
{
    float x;     // The x-coordinate of the point
    float y;     // The y-coordinate of the point
    int cluster; // The cluster that the point belongs to
} point;

int N = 0;                   // number of entries in the data
int k = 9;                   // number of centroids
point data[MAX_POINTS];      // Data coordinates
point cluster[MAX_CLUSTERS]; // The coordinates of each cluster center (also called centroid)

// File paths
char default_results_path[41] = "./../computed_results/kmeans-results.txt";
char default_input_path[64] = "./src/kmeans-data.txt";
char *results_path = default_results_path;
char *input_path = default_input_path;

void read_data()
{
    char line[256];
    FILE *fp;
    if ((fp = fopen(input_path, "r")) == NULL)
    {
        perror("Cannot open file");
        exit(EXIT_FAILURE);
    }

    // Initialize points from the data file
    while (fgets(line, sizeof(line), fp))
    {
        if (!isspace(line[0]) && N < MAX_POINTS) // Lines cannot start with whitespace
        {
            char data_buf[256] = {0};
            strncpy(data_buf, line, strlen(line) - 2);         // Copy everything except trailing '\n\0'
            sscanf(data_buf, "%f %f", &data[N].x, &data[N].y); // Save to data array
            data[N].cluster = -1;                              // Initialize the cluster number to -1
            N++;
        }
    }

    printf("Read the problem data!\n");
    // Initialize centroids randomly
    srand(0); // Setting 0 as the random number generation seed
    for (int i = 0; i < k; i++)
    {
        int r = rand() % N;
        cluster[i].x = data[r].x;
        cluster[i].y = data[r].y;
    }
    fclose(fp);
}

// Read command line arguments
void read_options(int argc, char *argv[])
{
    char *prog;
    prog = *argv;

    while (++argv, --argc > 0)
        if (**argv == '-')
            switch (*++*argv)
            {
            case 'f':
                --argc;
                input_path = *++argv;
                break;

            case 'k':
                --argc;
                int value = atoi(*++argv);
                if (value > MAX_CLUSTERS)
                {
                    k = MAX_CLUSTERS;
                }
                else if (value < 1)
                {
                    k = 1;
                }
                else
                {
                    k = value;
                }
                break;

            case 'p':
                --argc;
                // Where the server wants to save the results
                results_path = *++argv;
                break;

            default:
                printf("%s: ignored option: -%s\n", prog, *argv);
                printf("\nUsage: kmeans\n");
                printf("                [-f filename]    input data file\n");
                printf("                [-k clusters]    number of clusters\n");
                break;
            }
}

int get_closest_centroid(int i, int k)
{
    /* find the nearest centroid */
    int nearest_cluster = -1;
    double xdist, ydist, dist, min_dist;
    min_dist = dist = INT_MAX;
    for (int c = 0; c < k; c++)
    { // For each centroid
        // Calculate the square of the Euclidean distance between that centroid and the point
        xdist = data[i].x - cluster[c].x;
        ydist = data[i].y - cluster[c].y;
        dist = xdist * xdist + ydist * ydist; // The square of Euclidean distance
        // printf("%.2lf \n", dist);
        if (dist <= min_dist)
        {
            min_dist = dist;
            nearest_cluster = c;
        }
    }
    // printf("-----------\n");
    return nearest_cluster;
}

bool assign_clusters_to_points()
{
    bool something_changed = false;
    int old_cluster = -1, new_cluster = -1;
    for (int i = 0; i < N; i++)
    { // For each data point
        old_cluster = data[i].cluster;
        new_cluster = get_closest_centroid(i, k);
        data[i].cluster = new_cluster; // Assign a cluster to the point i
        if (old_cluster != new_cluster)
        {
            something_changed = true;
        }
    }
    return something_changed;
}

void update_cluster_centers()
{
    /* Update the cluster centers */
    int c;
    int count[MAX_CLUSTERS] = {0}; // Array to keep track of the number of points in each cluster
    point temp[MAX_CLUSTERS] = {0.0};

    for (int i = 0; i < N; i++)
    {
        c = data[i].cluster;
        count[c]++;
        temp[c].x += data[i].x;
        temp[c].y += data[i].y;
    }
    for (int i = 0; i < k; i++)
    {
        cluster[i].x = temp[i].x / count[i];
        cluster[i].y = temp[i].y / count[i];
    }
}

int kmeans(int k)
{
    bool somechange;
    int iter = 0;
    do
    {
        iter++; // Keep track of number of iterations
        somechange = assign_clusters_to_points();
        update_cluster_centers();
    } while (somechange);
    printf("Number of iterations taken = %d\n", iter);
    printf("Computed cluster numbers successfully!\n");
}

void write_results()
{
    FILE *fp = fopen(results_path, "w");
    if (fp == NULL)
    {
        perror("Cannot open the file");
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < N; i++)
    {
        fprintf(fp, "%.2f %.2f %d\n", data[i].x, data[i].y, data[i].cluster);
    }
    printf("Wrote the results to a file!\n");
}

int main(int argc, char *argv[])
{
    read_options(argc, argv);
    read_data();
    kmeans(k);
    write_results();
}
