#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]

use embedded_alloc::Heap;

#[global_allocator]
static HEAP: Heap = Heap::empty();

use teensy4_panic as _;

#[rtic::app(device = teensy4_bsp, peripherals = true, dispatchers = [GPT2])]
mod app {
    use swinder::{Gpio1, Gpio2, Gpio3, Gpio4, PitDelay, Delay2, GPT_FREQUENCY};
    
    #[local]
    struct Local {
    }
    
    #[shared]
    struct Shared {
        }
    
    #[init]
    fn init(ctx: init::Context) -> (Shared, Local) {
        
        (Shared {},
         Local {})
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        loop {
            cortex_m::asm::wfi();
        }
    }
    
    #[task(priority = 1)]
    async fn temp(_ctx: temp::Context) {

        loop {
            
        }
    }
}