# Classless Inter-Domain Routing Calculator

Gianfranco Fertino Hwandiano / 13515118


## Getting Started

1. Right click on main directory

2. Open terminal (Linux)

3. Type 'make' and enter

4. Type './client' to run the program

## Code explanation

### Phase 1

1. Determine the prefix length of the subnet mask. In this case, i picked 24.

2. Create the mask : 255.255.255.0

3. Mask the host address with the newly created subnet mask using operator &.

4. Add prefix length '/24' at the end of the result.

### Phase 2

1. Extract the prefix length from subnet address.

2. The available host address can be calculated by 2 ^ (32 - prefix)

### Phase 3

1. Extract the prefix length from subnet address.

2. Create subnet mask address using bitwise operation.  

3. Extract subnet address (eleminate the prefix).

4. Convert subnet, host, and mask address from string to internet address or sockaddr_in in C.

5. Mask the host address with subnet mask using operator &. Apply this to subnet address as well.

6. COmpare both of those to string and compare them. If they're identical then print 1 else print 0.
