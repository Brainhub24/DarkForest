#pragma once


#include "multiboot.h"
#include "types.h"
#include "stdlib.h"
#include "bits.h"
#include "Kassert.h"
#include "MM/MM_types.h"
#include "types.h"
#include "PageDirectory.h"

/*
    Virtual Memory map:
    (0MB-4MB - identity mapped)
	2MB - 3MB: page directories for tasks
    3MB - 4MB: kmalloc_eternal
	4MB-PAGE_SIZE...4MB - TempMap page (for a way to access a physical address)

	4MB - 0xc0000000 - User space

    0xc0000000 - 0xffffffff : kernel address space
    0xc0000000 - 0xc0ffffff : kernel image (code + data)
    0xc1000000 - 0xc5000000 : kernel heap
	0xd0000000 - 0xe0000000 - kernel stacks (a kernel stack per task)
*/

constexpr u32 IDENTITY_MAP_END = 4*MB;
constexpr u32 USERSPACE_START = 4*MB;
constexpr u32 KERNELSPACE_START = 0xc0000000;


// each PDE entry is responsible for 4MB of memory
constexpr u32 PDE_MAP_SIZE = 4 * MB;


#define ERR_NO_FREE_FRAMES 1

constexpr u32 TEMPMAP_ADDR = (u32)((4*MB) - PAGE_SIZE);


bool address_in_user_space(VirtualAddress);
bool address_in_kernel_space(VirtualAddress);

class MemoryManager {
private:
	MemoryManager(); // singelton
	~MemoryManager();
    void init(multiboot_info_t* mbt);


public:
    static void initialize(multiboot_info_t* mbt);
	static MemoryManager& the(u32 cr3 = 0);

	VirtualAddress temp_map(PhysicalAddress addr);
	void un_temp_map();
	void allocate(VirtualAddress virt_addr, bool writable, bool user_allowed);
	void disable_page(Frame frame);
	void deallocate(VirtualAddress virt_addr, bool free_page=true);
	void flush_tlb(VirtualAddress addr);
	void flush_entire_tlb();
	/**
	 * Returns the Page table Entry for given virtual address
	 * - If create_new=true, and there is no Page Directory Entry for the address,
	 *   a new page table will be created
	 * -  if tempMap_PageTable=true, the Page Table responsible for this address will be temp_mapped to virtual address so it can be accessed.
	 * 
	 * Note: if tempMap_PageTable=true, you should make sure to call un_temp_map after you're done
	 *       manipulating the PTE
	 */
    PTE ensure_pte(VirtualAddress addr,
			 bool create_new_PageTable=true,
			 bool tempMap_pageTable=true
			 );
	PDE get_pde(VirtualAddress virt_addr);
	PTE get_pte(VirtualAddress virt_addr, const PDE& pde);

	Frame get_free_frame(Err&, bool set_used=true);
	void set_frame_used(const Frame& frame);

	void set_frame_available(Frame frame);
	bool is_frame_available(const Frame frame);

	/**
	 * Return address of a new, cloned page directory
	 * The page directory is cloned from the current page directory
	 * Page tables are also clones, but the physical frames they point to are NOT cloned
	 */
	PageDirectory clone_page_directory();

	void copy_from_physical_frame(PhysicalAddress src, u8* dst);
	void copy_to_physical_frame(PhysicalAddress dst, u8* src);
	void memcpy_frames(PhysicalAddress dst, PhysicalAddress src);

	// after calling this method
	// it will not be possible anymore to create new kernel-space PDEs
	// this is useful to avoid discrepancies between kernel-sapce
	// memory across different tasks (because all tasks share the same kernel-space page tables)
	// if we want to disable this limitation in the future,
	// we will need to implement a mechanism to sync kernel-space PDEs between tasks
	void lock_kernel_PDEs() {m_kernel_PDEs_locked = true;}

private:
	u32 m_frames_avail_bitmap[N_FRAME_BITMAP_ENTRIES];
	PageDirectory* m_page_directory;
	bool m_tempmap_used;
	bool m_kernel_PDEs_locked;

};