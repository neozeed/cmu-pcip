/* Copyright 1986 by Carnegie Mellon */
/*  Copyright 1983, 1985 by the Massachusetts Institute of Technology  */

/* information common to all ethernet drivers
*/

/* initialization modes: 0 = rcv normal + broadcast packets */
#define	LOOPBACK	0x01	/* send packets in loopback mode */
#define	ALLPACK		0x02	/* receive all packets: promiscuous */
#define	MULTI		0x04	/* receive multicast packets */

/* ethernet packet header
*/
struct ethhdr {
	char	e_dst[6];
	char	e_src[6];
	unsigned	e_type;
	};

/* per-net structure containing ethernet-specific information
*/
struct ether_info {
	char	et_my_address[6];	/* address of this interface */
	char	*et_driver_data;	/* driver specific data */
	};

/* ethernet packet types: not all of these are really used.
	These packet types are ALREADY BYTESWAPPED.
*/
#define	ET_IP		0x0008	/* really 0x800 */
#define	ET_ARP		0x0608	/* really 0x806 */
#define	ET_CHAOS	0x0408	/* really 0x804 */

/* minimum length legal ethernet packet
*/
#define	ET_MINLEN	60

extern char ETBROADCAST[];
