use teensy4_pins::t41::*;
use teensy4_bsp::board::{self, PERCLK_FREQUENCY};
use teensy4_bsp::hal::{
    gpio::{Input, Output, Port},
    gpt::{Gpt1, Gpt2},
    pit::Pit2,
    timer::Blocking,
};

pub const GPT_FREQUENCY: u32 = 1_000;

/// The general-purpose delay shared by different peripherals
pub type Delay2 = Blocking<Gpt2, GPT_FREQUENCY>;
/// The PIT-defined delay for initializing the IMU.
pub type PitDelay = Blocking<Pit2, PERCLK_FREQUENCY>;
/// The first GPIO port
pub type Gpio1 = Port<1>;
/// The second GPIO port
pub type Gpio2 = Port<2>;
/// The third GPIO port
pub type Gpio3 = Port<3>;
/// The fourth GPIO port
pub type Gpio4 = Port<4>;