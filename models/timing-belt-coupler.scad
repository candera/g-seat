use <Pulley_T-MXL-XL-HTD-GT2_N-tooth.scad>;

tooth_pitch = 5.0;
num_teeth = 5;
height = 12;
belt_thickness = 3.8; 
wall_thickness = 5;
screw_diameter = 2.5;

translate ([0,0,wall_thickness]) {
  difference () {
    union () {
      vertical(num_teeth, tooth_pitch, height, wall_thickness);
      translate([0, belt_thickness + wall_thickness, 0])
        vertical(num_teeth, tooth_pitch, height, wall_thickness);
      translate([0, 2 * (belt_thickness + wall_thickness), 0])
        wall(num_teeth, tooth_pitch, height, wall_thickness);
      translate([0,0,-wall_thickness])
        cap(num_teeth, tooth_pitch, belt_thickness, wall_thickness);
      translate([0,width(belt_thickness,wall_thickness) + wall_thickness,-wall_thickness]) {
        cap(num_teeth, tooth_pitch, belt_thickness, wall_thickness);
      }
    }
    translate([0,width(belt_thickness,wall_thickness) + wall_thickness,-wall_thickness])
      holes(screw_diameter+0.2, wall_thickness, num_teeth, tooth_pitch, belt_thickness, wall_thickness);
    translate([0,0,-wall_thickness])
      holes(screw_diameter, height + 2 * wall_thickness, num_teeth, tooth_pitch, belt_thickness, wall_thickness);
  }
}

function width(belt_thickness, wall_thickness) =
  (2 * belt_thickness) + (3 * wall_thickness);

module holes(diameter, depth, count, pitch, belt_thickness, wall_thickness) {
  length = count * pitch;
  union () {
    translate([length * 0.1, wall_thickness / 2, -0.01])
      cylinder($fn=12, h=depth+0.02, r1=diameter/2 + 0.1, r2=diameter/2 + 0.1);
    translate([length * 0.9, wall_thickness / 2, -0.01])
      cylinder($fn=12, h=depth+0.02, r1=diameter/2 + 0.1, r2=diameter/2 + 0.1);
    translate([length * 0.1,
               width(belt_thickness, wall_thickness) - wall_thickness / 2,
               -0.01])
      cylinder($fn=12, h=depth+0.02, r1=diameter/2 + 0.1, r2=diameter/2 + 0.1);
    translate([length * 0.9,
               width(belt_thickness, wall_thickness) - wall_thickness / 2,
               -0.01])
      cylinder($fn=12, h=depth+0.02, r1=diameter/2 + 0.1, r2=diameter/2 + 0.1);
  }
}
    
module cap(count, pitch, belt_thickness, wall_thickness) {
  cube([count * pitch,
        (2 * belt_thickness) + (3 * wall_thickness),
        wall_thickness]);

}

module teeth(count, pitch, height) {
  for ( i = [0:count-1] ) {
    translate([1.89036+(i*pitch), 0, 0])
      //XL(height);
      HTD_5mm(height);
  }
}

module wall(count, pitch, height, wall_thickness, screw_diameter) {
  cube([count*pitch, wall_thickness, height+2]);
}

module vertical(count, pitch, height, wall_thickness) {
  translate ([0, wall_thickness, 0])
    union () {
    teeth(count, pitch, height);
    translate([0,-wall_thickness,0])
      wall(count, pitch, height, wall_thickness);

  }
}

