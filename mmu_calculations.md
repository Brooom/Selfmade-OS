# MMU
## Variables
size of the memory region: somr = 64-16 = 48  
page size: ps = 4 kiB  
table entry size tes = 64 bit = 8 byte = 2^3 B

## Calculation
Virtual Address Space $= 2^\text{somr}=2^{48}=256 \text{ TiB}$  
Nr of pages $= 2^{48}/2^{12}=2^{36}$  
This does not fit into one page table.  
Nr of page table entries $=2^{12}/2^{3}=2^{9}$  

Nr of L3 page tables $= 2^{36}/2^{9}=2^{27}$  
Nr of L2 page tables $= 2^{27}/2^{9}=2^{18}$  
Nr of L1 page tables $= 2^{18}/2^{9}=2^{9}$  
Nr of L0 page tables $= 2^{9}/2^{9}=1$  

