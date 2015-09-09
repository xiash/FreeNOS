/*
 * Copyright (C) 2015 Niek Linnenbank
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __KERNEL_H
#define __KERNEL_H

#include <Macros.h>
#include <Types.h>
#include <Singleton.h>
#include <BootImage.h>
#include <Memory.h>
#include "Process.h"
#include "ProcessManager.h"

/** Forward declarations. */
class API;
class SplitAllocator;
class IntController;
struct CPUState;

/**
 * Function which is called when the CPU is interrupted. 
 *
 * @param state State of the CPU on the moment the interrupt occurred. 
 * @param param Optional parameter for the handler. 
 */

// TODO: move this to libarch's IntController? The IntController
// could take care of invocing a certain ISR when interrupt is raised.

typedef void InterruptHandler(struct CPUState *state, ulong param);
    
/**
 * Interrupt hook class.
 */
typedef struct InterruptHook
{
    /**
     * Constructor function.
     * @param h Handler function for the hook.
     * @param p Parameter to pass.
     */
    InterruptHook(InterruptHandler *h, ulong p) : handler(h), param(p)
    {
    }

    /**
     * Comparision operator.
     * @param i InterruptHook pointer.
     * @return True if equal, false otherwise.
     */
    bool operator == (InterruptHook *i)
    {
        return handler == i->handler && param == i->param;
    }

    /** Executed at time of interrupt. */
    InterruptHandler *handler;

    /** Passed to the handler. */
    ulong param;
}
InterruptHook;

/** 
 * @defgroup kernel kernel (generic)
 * @{ 
 */

/**
 * FreeNOS kernel implementation.
 */
class Kernel : public Singleton<Kernel>
{
  public:

    /**
     * Result codes.
     */
    enum Result
    {
        Success,
        InvalidBootImage,
        ProcessError
    };

    /**
     * Constructor function.
     *
     * @param kernel Describes the start and end of the kernel program in memory.
     * @param memory Describes the start and end of physical RAM in the system.
     */
    Kernel(Memory::Range kernel, Memory::Range memory);

    /**
     * Initialize heap.
     *
     * This function sets up the kernel heap for
     * dynamic memory allocation with new() and delete()
     * operators. It must be called before any object
     * is created using new().
     *
     * @return Zero on success or error code on failure.
     */
    static Error heap(Address base, Size size);

    /**
     * Get physical memory allocator.
     *
     * @return SplitAllocator object pointer
     */
    SplitAllocator * getAllocator();

    /**
     * Get process manager.
     *
     * @return Kernel ProcessManager object pointer.
     */
    ProcessManager * getProcessManager();

    /**
     * Get API.
     *
     * @return Kernel API object pointer.
     */
    API * getAPI();

    /**
     * Get the current MMU context.
     *
     * @return MemoryContext object pointer
     */
    MemoryContext * getMemoryContext();

    /**
     * Execute the kernel.
     */
    int run();

    /** 
     * Enable or disable an hardware interrupt (IRQ). 
     *
     * @param irq IRQ number. 
     * @param enabled True to enable, and false to disable. 
     */
    void enableIRQ(u32 irq, bool enabled);

    /**
     * Hooks a function to an hardware interrupt.
     *
     * @param vec Interrupt vector to hook on.
     * @param h Handler function.
     * @param p Parameter to pass to the handler function.
     */
    virtual void hookIntVector(u32 vec, InterruptHandler h, ulong p);

    /**
     * Execute an interrupt handler.
     *
     * @param vec Interrupt vector.
     * @param state CPU state.
     */
    virtual void executeIntVector(u32 vec, CPUState *state);

    /**
     * Loads the boot image.
     */
    virtual Result loadBootImage();

  private:

    /**
     * Load a boot program.
     *
     * @param image BootImage pointer loaded by the bootloader in kernel virtual memory.
     * @param imagePAddr Physical memory address of the boot image.
     * @param index Index in the BootProcess table.
     */
    virtual Result loadBootProcess(BootImage *image, Address imagePAddr, Size index);

  protected:

    /** Physical memory allocator */
    SplitAllocator *m_alloc;

    /** Process Manager */
    ProcessManager *m_procs;

    /** API handlers object */
    API *m_api;

    /** Interrupt handlers. */
    Vector<List<InterruptHook *> *> m_interrupts;

    /** Interrupt Controller. */
    IntController *m_intControl;
};

/**
 * @}
 */

#endif /* __KERNEL_H */
