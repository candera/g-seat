brace_length = 15;
bracket_diameter = 90;
bracket_length = 6;
mounting_plate_width = 5;
mounting_screws = 3;
mounting_screw_location_radius = 22.5;
mounting_screw_diameter = 2;
hollow_arm_length = 50;
hollow_interior_width = 2;
hollow_interior_length = 4;
hollow_arm_fan_mount_factor = 0.75;
wall_thickness = 1.5;
epsilon = 0.05;
motor_diameter = 60;
motor_barrel_length = 60;
clamp_gap = 2;
clamp_arm_length = 10;
clamp_arm_clearance_hole_diameter=3.2;
clamp_arm_pilot_hole_diameter=2.7;
clearance=0.7;


module fan_mount() {
  union () {
    // Circular walls
    difference() {
      cylinder($fn=96,d=bracket_diameter,h=bracket_length);
      translate([0,0,-epsilon]) {
        cylinder($fn=96,r=bracket_diameter/2-wall_thickness,h=bracket_length+2*epsilon);
      }
    }
    // Base plate
    difference() {
      translate([-bracket_diameter/2,-bracket_diameter/2,0]) {
        cube([bracket_diameter, bracket_diameter, bracket_length]);
      }
      translate([0,0,-epsilon]) {
        cylinder($fn=96,r=bracket_diameter/2-wall_thickness,h=bracket_length+2*epsilon);
      }
    }
    // Hollow supports
    x_offset = motor_barrel_length / 2;
    y_offset = bracket_diameter/2 - hollow_interior_length/2 - wall_thickness/2;
    for (x = [-x_offset,x_offset],
           y = [-y_offset,y_offset]) {
      translate([x,y,hollow_arm_length/2]) {
        difference() {
          cube([hollow_interior_width+wall_thickness, hollow_interior_length+wall_thickness, hollow_arm_length], center=true);
          translate([0, 0, epsilon+wall_thickness/2]) {
            cube([hollow_interior_width, hollow_interior_length, hollow_arm_length-wall_thickness], center=true);
          }
        }
      }
    }
    // End arms
    difference() {
      for (i = [1:mounting_screws]) {
        rotate([0,0,i*360/mounting_screws]) {
          translate([0,-mounting_plate_width/2,0]) {
            cube([bracket_diameter/2,mounting_plate_width,wall_thickness]);
          }
        }
      }
      for (i = [1:mounting_screws]) {
        rotate([0,0,i*360/mounting_screws]) {
          translate([mounting_screw_location_radius,0,-epsilon]) {
            cylinder($fn=12,h=wall_thickness+2*epsilon,d=mounting_screw_diameter);
          }
        }
      }
    }
    // Braces
    for (i = [1:mounting_screws]) {
      rotate([0,0,i*360/mounting_screws]) {
        translate([bracket_diameter/2-brace_length,-wall_thickness/2,0]) {
          cube([brace_length,wall_thickness,bracket_length]);
        }
      }
    }
  }
}

module motor_mount() {
  union() {
    // Cylindrical walls
    difference() {
      cylinder($fn=96,r=motor_diameter/2,h=bracket_length);
      translate([0,0,-epsilon]) {
        cylinder($fn=96,r=motor_diameter/2-wall_thickness,h=bracket_length+2*epsilon);
      }
      // Gap
      translate([motor_diameter/2-wall_thickness-epsilon,-clamp_gap/2,-epsilon]) {
        cube([wall_thickness+2*epsilon, clamp_gap, bracket_length+2*epsilon]);
      }
    }
    difference() {
      // Clamp arms
      for (y = [-clamp_gap/2-wall_thickness, clamp_gap/2]) {
        translate([motor_diameter/2-wall_thickness,y,0]) {
          #cube([clamp_arm_length,wall_thickness,bracket_length]);
        }
      }
      // Screw holes
      translate([motor_diameter/2+clamp_arm_length/2,-clamp_gap/2+epsilon,bracket_length/2]) {
        rotate([90,0,0]) {
          cylinder($fn=24,d=clamp_arm_clearance_hole_diameter,h=wall_thickness+2*epsilon);
        }
      }
      translate([motor_diameter/2+clamp_arm_length/2,clamp_gap/2+epsilon+wall_thickness,bracket_length/2]) {
        rotate([90,0,0]) {
          cylinder($fn=24,d=clamp_arm_pilot_hole_diameter,h=wall_thickness+2*epsilon);
        }
      }

    }
    // Base
    difference() {
      linear_extrude(height=hollow_interior_width-clearance) {
        polygon(points=[[0,motor_diameter/2],
                        [-motor_diameter/2,bracket_diameter/2-clearance],
                        [-motor_diameter/2,-bracket_diameter/2+clearance],
                        [0,-motor_diameter/2]]);
      }
      translate([0,0,-epsilon]) {
        cylinder($fn=96,r=motor_diameter/2-wall_thickness,h=bracket_length+2*epsilon);
      }
    }
    // Arms
    for (y=[-bracket_diameter/2+wall_thickness/2+clearance/2,bracket_diameter/2-wall_thickness/2-hollow_interior_length+clearance/2]) {
      translate([-hollow_arm_length*hollow_arm_fan_mount_factor-motor_diameter/2,y,0]) {
        cube([hollow_arm_length*hollow_arm_fan_mount_factor,hollow_interior_length-clearance,hollow_interior_width-clearance]);
      }
    }
  }
}

module both() {
  union() {
    fan_mount();
    translate([bracket_diameter/2,0,hollow_arm_length]) {
      rotate([0,90,0]) {
        color([1,0,0,0.5]) {
          motor_mount();
        }
      }
    }
  }
}

//motor_mount();
fan_mount();
