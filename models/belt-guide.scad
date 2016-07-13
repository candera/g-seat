wall_thickness = 2;
belt_width = 50;
center_gap = 10;
width = 35;
epsilon = 0.01;
flange_length = 20;
flange_width = 10;
screw_hole_diameter = 5;

union() {
  difference () {
    // Mounting flange
    translate ([0, -flange_length, 0]) {
      cube ([flange_width, belt_width + 2 * wall_thickness + 2 * flange_length, wall_thickness]);
    }
    // Screw hole #1
    translate([flange_width/2, -flange_length/2, -epsilon]) {
      cylinder($fn=24, h=wall_thickness+2*epsilon, d=screw_hole_diameter);
    }
    // Screw hole #2
    translate([flange_width/2, flange_length/2 + belt_width + 2 * wall_thickness, -epsilon]) {
      cylinder($fn=24, h=wall_thickness + 2 * epsilon, d=screw_hole_diameter);
    }

  }
  difference() {
    cube([width, belt_width + 2 * wall_thickness, wall_thickness * 3]);
    translate([-epsilon, wall_thickness, wall_thickness]) {
      cube([width + 2*epsilon, belt_width, wall_thickness]);
    }
    // Gap
    translate([-epsilon, belt_width/2+wall_thickness - center_gap / 2, wall_thickness*2 - epsilon]) {
      cube([width+2*epsilon, center_gap, wall_thickness + 2*epsilon]);
    }
  }
}
