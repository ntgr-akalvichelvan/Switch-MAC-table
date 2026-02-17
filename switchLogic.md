switch

1) parse through incoming frame
2) Identify destination MAC and Source MAC
3) Maintain a MAC table,that has contents about mac address and port number
4) Ageing -> purging any entry that is older than 300secs

when a frame enters switch

1) Dest MAC read,
2) Dest MAC is compared with entries in the MAC table
3) It read Source MAC and populates the MAC table entry with Source MAC and port number.
3) From the MAC table appropriate Port is selected
4) frame is sent or switched to that port
5) Dest MAC not found through the MAC table -> flooded
