1. Refactor load_cluster_chain to appear in stage0.asm
2. Make load_cluster_chain self sustaining
3. Make sure to find the first cluster of the kernel while in stage 0 (to mot get the root dir again)
4. Make sure disk_read is self sustaining
5. Load the kernel.asm
6. Create a C file
7. Link together