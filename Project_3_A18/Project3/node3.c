#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "project3.h"

#define INFINITY 9999

extern int TraceLevel;
extern int clocktime;

struct distance_table {
  int costs[MAX_NODES][MAX_NODES];
};

/////////////////////////////////////////////////////////////////////
//  printdt
//  This routine is being supplied to you.  It is the same code in
//  each node and is tailored based on the input arguments.
//  Required arguments:
//  MyNodeNumber:  This routine assumes that you know your node
//                 number and supply it when making this call.
//  struct NeighborCosts *neighbor:  A pointer to the structure 
//                 that's supplied via a call to getNeighborCosts().
//                 It tells this print routine the configuration
//                 of nodes surrounding the node we're working on.
//  struct distance_table *dtptr: This is the running record of the
//                 current costs as seen by this node.  It is 
//                 constantly updated as the node gets new
//                 messages from other nodes.
/////////////////////////////////////////////////////////////////////
void printdt3( int MyNodeNumber, struct NeighborCosts *neighbor, 
        struct distance_table *dtptr ) {
    int       i, j;
    int       TotalNodes = neighbor->NodesInNetwork;     // Total nodes in network
    int       NumberOfNeighbors = 0;                     // How many neighbors
    int       Neighbors[MAX_NODES];                      // Who are the neighbors

    // Determine our neighbors 
    for ( i = 0; i < TotalNodes; i++ )  {
        if (( neighbor->NodeCosts[i] != INFINITY ) && i != MyNodeNumber )  {
            Neighbors[NumberOfNeighbors] = i;
            NumberOfNeighbors++;
        }
    }
    // Print the header
    printf("                via     \n");
    printf("   D%d |", MyNodeNumber );
    for ( i = 0; i < NumberOfNeighbors; i++ )
        printf("     %d", Neighbors[i]);
    printf("\n");
    printf("  ----|-------------------------------\n");

    // For each node, print the cost by travelling thru each of our neighbors
    for ( i = 0; i < TotalNodes; i++ )   {
        if ( i != MyNodeNumber )  {
            printf("dest %d|", i );
            for ( j = 0; j < NumberOfNeighbors; j++ )  {
                    printf( "  %4d", dtptr->costs[i][Neighbors[j]] );
            }
            printf("\n");
        }
    }
    printf("\n");
}    // End of printdt3
struct distance_table dt3;
struct NeighborCosts   *neighbor3;
//int const BYZEROID = 2; // id of the source for things like arrays that start with 0

/* students to write the following two routines, and maybe some others */

void rtinit3() {
     printf("\n\n!!!---Init...---!!!\n\n");
    int SOURCEID = 3; // id of the source
    int i, j, initx, inity, x, y;
    int destID;
    struct RoutePacket *packet = malloc(sizeof(struct RoutePacket));

    /* Get the costs of neighbors */
    neighbor3 = malloc(sizeof(struct NeighborCosts));
    memcpy(neighbor3, getNeighborCosts(SOURCEID), sizeof(struct NeighborCosts));

    /* initialize distance table */
    for(initx = 0; initx < MAX_NODES; initx++) {
        for(inity = 0; inity < MAX_NODES; inity++) { dt3.costs[initx][inity] = INFINITY; }
    }

    /* Update distance table to reflect new values */
    for(i = 0; i < MAX_NODES; i++) {
        printf("iteration: %i\n", i);
        printf("value of neighbor2: %d\n", neighbor3->NodeCosts[i]);
        
        memcpy(&dt3.costs[i][i], &neighbor3->NodeCosts[i], sizeof(int));
        printf("value of dt3: %d\n", dt3.costs[i][i]);

        printf("printing dt costs changes...\n");
        for(j = 0; j < neighbor3->NodesInNetwork; j++) {
            printf("Dt3cost[%d]: %d\nNeighbor costs[%d]: %d\n", j,  dt3.costs[i][j], j, neighbor3->NodeCosts[j]);
        }
    }

    for(y = 0; y < MAX_NODES; y++) {
        destID = y;
        if(destID != SOURCEID) {
            /* Create a packet and send to layer 2 */
            packet->sourceid = SOURCEID;
            packet->destid = destID;
            /* Transfer costs into packet */
            for(x = 0; x < MAX_NODES; x++) {
                packet->mincost[x] = dt3.costs[x][x]; // does this work
            }
            toLayer2(*packet);
            printf("\nAt time: %ld\n", clocktime);
            printf("Packet sent to %d by %d\n", packet->destid, packet->sourceid);
        }
    }
    printf("\nAt time: %ld\n", clocktime);
    printdt3(SOURCEID, neighbor3, &dt3);
}


void rtupdate3( struct RoutePacket *rcvdpkt ) {
    printf("\n\n!!!---Updating...---!!!\n\n");
    int SOURCEID = 3; // id of the source
    /* Get the new distance table */
    int copy;
    int temp;
    int tempArray[MAX_NODES];
    int i,j,x,y;
    int destID;
    int flag = 0;
    struct RoutePacket *packet = malloc(sizeof(struct RoutePacket));
    //printf("Rcvd packet sourceid: %d", dt2.costs[rcvdpkt->sourceid][i]);
    printf("Made it\n");

    /* update distance table */
    for(i = 0; i < MAX_NODES; i++) {
        if(i != SOURCEID) {
            if(dt3.costs[i][rcvdpkt->sourceid] > dt3.costs[rcvdpkt->sourceid][rcvdpkt->sourceid] + rcvdpkt->mincost[i]) { // check all nodes in row if there is a faster way
                dt3.costs[i][rcvdpkt->sourceid] = dt3.costs[rcvdpkt->sourceid][rcvdpkt->sourceid] + rcvdpkt->mincost[i];   
                flag = 1;
            }
        }
    }
    printf("Made it\n");

    for (j = 0; j < neighbor3->NodesInNetwork; j++) {
        printf("Yo\n");
        if(j != SOURCEID) {
            printf("j\n");

            /* Fine the smallest values */
            if(flag == 1) {         // If the table has changed
                printf("Yo\n");
            for(x = 0; x < MAX_NODES; x++) {
                temp = dt3.costs[x][0];         // temp is first value of each row
                for(y = 0; y < MAX_NODES; y++) {
                    if(temp > dt3.costs[x][y])  // if there is a smaller value in the row
                    {
                        temp = dt3.costs[x][y];
                    }
                }
                tempArray[x] = temp;            // add the smallest value of the row into tempArray
            }
            printf("Made it\n");

            /* Send packet to neighbors */
            packet->sourceid = SOURCEID;
            packet->destid = j;
            memcpy(&packet->mincost, &tempArray, sizeof(int)*4);
            toLayer2(*packet);                   // Send packet to all neighbors (the project will ignore packets that are not connected)
            }
        }
    }     

    /* start output trace */
    printf("\nAt time: %ld", clocktime);
    printf("Packet was recieved. Sender of packet: %d\nReceiver of packet: %d\n", rcvdpkt->sourceid, rcvdpkt->destid);
    printf("Updated distance table: \n");
    printdt3(3, neighbor3, &dt3);
}

