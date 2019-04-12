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
struct distance_table dt1;
struct NeighborCosts   *neighbor1;
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
void printdt1( int MyNodeNumber, struct NeighborCosts *neighbor, 
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
}    // End of printdt1

//int const BYZEROID = 0; // id of the source for things like arrays that start with 0

/* students to write the following two routines, and maybe some others */

/* This routine will be called once at the beginning of the emulation. rtinit0() has no arguments. It should initialize the distance table in 
node0 to reflect the direct costs to is neighbors by using GetNeighborCosts(). In Figure 1, 
representing the default configuration, all links are bi-directional and the costs in both directions are identical. 
After initializing the distance table, and any other data structures needed by your node0 routines, 
it should then send to its directly-connected neighbors (in the Figure, 1, 2 and 3) 
its minimum cost paths to all other network nodes. 
This minimum cost information is sent to neighboring nodes in a routing packet by calling the routine tolayer2(), as described below. 
The format of the routing packet is also described below. */
void rtinit1() {
    printf("\n\n!!!---Init...---!!!\n\n");
    int SOURCEID = 1; // id of the source
    int i, j, initx, inity, x, y;
    int destID;
    struct RoutePacket *packet = malloc(sizeof(struct RoutePacket));

    /* Get the costs of neighbors */
    neighbor1 = malloc(sizeof(struct NeighborCosts));
    memcpy(neighbor1, getNeighborCosts(SOURCEID), sizeof(struct NeighborCosts));

    /* initialize distance table */
    for(initx = 0; initx < MAX_NODES; initx++) {
        for(inity = 0; inity < MAX_NODES; inity++) { dt1.costs[initx][inity] = INFINITY; }
    }

    /* Update distance table to reflect new values */
    for(i = 0; i < MAX_NODES; i++) {
        printf("iteration: %i\n", i);
        printf("value of neighbor1: %d\n", neighbor1->NodeCosts[i]);
        
        memcpy(&dt1.costs[i][i], &neighbor1->NodeCosts[i], sizeof(int));
        printf("value of dt1: %d\n", dt1.costs[i][i]);

        printf("printing dt costs changes...\n");
        for(j = 0; j < neighbor1->NodesInNetwork; j++) {
            printf("Dt1cost[%d]: %d\nNeighbor costs[%d]: %d\n", j,  dt1.costs[i][j], j, neighbor1->NodeCosts[j]);
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
                packet->mincost[x] = dt1.costs[x][x]; // does this work
            }
            toLayer2(*packet);
            printf("\nAt time: %ld :: \n", clocktime);
            printf("Packet sent to %d by %d\n", packet->destid, packet->sourceid);
        }
    }
    printf("\nAt time: %ld\n", clocktime);
    printdt1(SOURCEID, neighbor1, &dt1);
}

/*This routine will be called when node 0 receives a routing packet that was sent to it by one of its directly connected neighbors. The parameter *rcvdpkt is a pointer to the packet 
that was received rtupdate0() is the "heart" of the distance vector algorithm. 
The values it receives in a routing packet from some other node i contain i's 
current shortest path costs to all other network nodes. rtupdate0() uses these received values to update its own distance 
table (as specified by the distance vector algorithm). If its own minimum cost to another node changes as a 
result of the update, node 0 informs its directly connected neighbors of this change in minimum cost by sending them a routing packet. 
Recall that in the distance vector algorithm, only directly connected nodes will exchange routing packets. 
Thus, for the example in Figure 1, nodes 1 and 2 will communicate with each other, 
but nodes 1 and 3 will not communicate with each other. As we saw/will see in class, 
the distance table inside each node is the principal data structure used by the distance vector algorithm. You will find it 
convenient to declare the distance table as a N-by-N array of int's, where entry [i,j] in the distance table in node 0 is node 0's 
currently computed cost to node i via direct neighbor j. If 0 is not directly connected to j, you can ignore this entry. 
We will use the convention that the integer value 9999 is ``infinity.'' */
void rtupdate1( struct RoutePacket *rcvdpkt ) {
    printf("\n\n!!!---Updating...---!!!\n\n");
    int SOURCEID = 1; // id of the source
    /* Get the new distance table */
    int copy;
    int temp;
    int tempArray[MAX_NODES];
    int i,j,x,y;
    int destID;
    int flag = 0;
    struct RoutePacket *packet = malloc(sizeof(struct RoutePacket));
    /* update distance table */
    for(i = 0; i < MAX_NODES; i++) {
        if(i != SOURCEID) {
            if(dt1.costs[i][rcvdpkt->sourceid] > dt1.costs[rcvdpkt->sourceid][rcvdpkt->sourceid] + rcvdpkt->mincost[i]) { // check all nodes in row if there is a faster way
                printf("Distance table updated!!!\n");
                dt1.costs[i][rcvdpkt->sourceid] = dt1.costs[rcvdpkt->sourceid][rcvdpkt->sourceid] + rcvdpkt->mincost[i];   
                flag = 1;
            }
        }
    }
    for (j = 0; j < neighbor1->NodesInNetwork; j++) {
        if(j != SOURCEID) {
            /* Fine the smallest values */
            if(flag == 1) {         // If the table has changed
            for(x = 0; x < MAX_NODES; x++) {
                temp = dt1.costs[x][0];         // temp is first value of each row
                for(y = 0; y < MAX_NODES; y++) {
                    if(temp > dt1.costs[x][y])  // if there is a smaller value in the row
                    {
                        temp = dt1.costs[x][y];
                    }
                }
                tempArray[x] = temp;            // add the smallest value of the row into tempArray
            }
            /* Send packet to neighbors */
            packet->sourceid = SOURCEID;
            packet->destid = j;
            memcpy(&packet->mincost, &tempArray, sizeof(int)*4);
            toLayer2(*packet);                   // Send packet to all neighbors (the project will ignore packets that are not connected)
            printf("Packet mincost sent by %d to %d:: ", packet->sourceid, packet->destid);
            int print;
            for(print = 0; print < MAX_NODES; print++)
            {
                printf("%d ", packet->mincost[print]);
            }
            printf("\n");
            }   
        }
    }     

    /* start output trace */
    printf("\nAt time: %ld :: ", clocktime);
    printf("Packet was recieved. Sender of packet: %d\nReceiver of packet: %d\n", rcvdpkt->sourceid, rcvdpkt->destid);
    printf("Updated distance table: \n");
    printdt1(1, neighbor1, &dt1);
}


