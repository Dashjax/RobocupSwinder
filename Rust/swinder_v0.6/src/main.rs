#![no_std]
#![no_main]
#![feature(type_alias_impl_trait)]

use embedded_alloc::Heap;

#[global_allocator]
static HEAP: Heap = Heap::empty();

use teensy4_panic as _;

#[rtic::app(device = teensy4_bsp, peripherals = true, dispatchers = [GPT2])]
mod app {
    use embedded_hal::digital::OutputPin;
    use teensy4_bsp as bsp;
    use bsp::board::{self, PERCLK_FREQUENCY};
    use bsp::hal::gpio::{Input, Output};
    use imxrt_hal::timer::Blocking;
    use rtic_monotonics::systick::*;
    use teensy4_pins::t41::*;
    //use stepper_driver::MotorDriver;
    use hd44780_driver::{HD44780, *};
    
    #[local]
    struct Local {
        led: board::Led,
        up_button: Input<P35>,
        down_button: Input<P36>,
        mid_button: Input<P37>,
        left_button: Input<P38>,
        right_button: Input<P39>,
        lcd_display: HD44780<bus::FourBitBus<imxrt_hal::gpio::Output<imxrt_hal::iomuxc::Pad<1075806408, 1075806904>>, imxrt_hal::gpio::Output<imxrt_hal::iomuxc::Pad<1075806404, 1075806900>>, imxrt_hal::gpio::Output<imxrt_hal::iomuxc::Pad<1075806244, 1075806740>>, imxrt_hal::gpio::Output<imxrt_hal::iomuxc::Pad<1075806248, 1075806744>>, imxrt_hal::gpio::Output<imxrt_hal::iomuxc::Pad<1075806252, 1075806748>>, imxrt_hal::gpio::Output<imxrt_hal::iomuxc::Pad<1075806260, 1075806756>>>>,
        coil_motor_step: Output<P24>,
        coil_motor_dir: Output<P25>
    }
    
    #[shared]
    struct Shared {
        pit_delay: Blocking<imxrt_hal::pit::Pit<3>, 1000000>,
        }
    
    #[init]
    fn init(ctx: init::Context) -> (Shared, Local) {
        let board::Resources {
            mut gpio1,
            mut gpio2,
            mut gpio3,
            mut gpio4,
            pins,
            usb,
            pit: (_pit0, _pit1, _pit2, pit3),
            ..
        } = board::t41(ctx.device);
        let led = board::led(&mut gpio2, pins.p13);
        let up_button = gpio2.input(pins.p35);
        let down_button = gpio2.input(pins.p36);
        let mid_button = gpio2.input(pins.p37);
        let left_button = gpio1.input(pins.p38);
        let right_button = gpio1.input(pins.p39);

        let rs_pin = gpio1.output(pins.p0);
        let en_pin = gpio1.output(pins.p1);
        let db4_pin = gpio4.output(pins.p2);
        let db5_pin = gpio4.output(pins.p3);
        let db6_pin = gpio4.output(pins.p4);
        let db7_pin = gpio4.output(pins.p5);
        let mut pit_delay = Blocking::<_, PERCLK_FREQUENCY>::from_pit(pit3);

        let coil_motor_step = gpio1.output(pins.p24);
        let coil_motor_dir = gpio1.output(pins.p25);
        coil_motor_dir.set();
        
        let mut lcd_display = HD44780::new_4bit(
            rs_pin,
            en_pin,
            db4_pin,
            db5_pin,
            db6_pin,
            db7_pin,
            &mut pit_delay,
        ).unwrap();
        
        bsp::LoggingFrontend::default_log().register_usb(usb);

        let systick_token = rtic_monotonics::create_systick_token!();
        Systick::start(ctx.core.SYST, 600_000_000, systick_token);

        lcd_display.reset(&mut pit_delay);

        // Clear existing characters
        lcd_display.clear(&mut pit_delay);

        // Display the following string
        lcd_display.write_str("Hello, world!", &mut pit_delay);
        
        blink_led::spawn().ok();
        (Shared {pit_delay
                            },
         Local {led,
                up_button,
                down_button,
                mid_button,
                left_button,
                right_button,
                lcd_display,
                coil_motor_step,
                coil_motor_dir})
    }

    #[idle]
    fn idle(_: idle::Context) -> ! {
        loop {
            cortex_m::asm::wfi();
        }
    }
    
    #[task(local = [led, up_button, coil_motor_step], shared = [pit_delay], priority = 1)]
    async fn blink_led(_ctx: blink_led::Context) {

        loop {

            if _ctx.local.up_button.is_set() {
                log::info!("Button Pressed!");
                for _x in 0..240 {
                    _ctx.local.led.set_high();
                    _ctx.local.coil_motor_step.set_high();
                    Systick::delay(100u32.micros()).await;
                    _ctx.local.led.set_low();
                    _ctx.local.coil_motor_step.set_low();
                    Systick::delay(100u32.micros()).await;
                }
            }
            
        }

        
    }
}