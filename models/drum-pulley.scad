use <Pulley_T-MXL-XL-HTD-GT2_N-tooth.scad>;

retainer = 1;           // Belt retainer above teeth, 0 = No, 1 = Yes
retainer_ht = 1.5;      // height of retainer flange over pulley, standard = 1.5
idler = 1;                      // Belt retainer below teeth, 0 = No, 1 = Yes
idler_ht = 1.5;         // height of idler flange over pulley, standard = 1.5

teeth = 94;                      // Number of teeth, standard Mendel T5 belt = 8, gives Outside Diameter of 11.88mm
profile = 10;            // 1=MXL 2=40DP 3=XL 4=H 5=T2.5 6=T5 7=T10 8=AT5 9=HTD_3mm 10=HTD_5mm 11=HTD_8mm 12=GT2_2mm 13=GT2_3mm 14=GT2_5mm

motor_shaft = 9.525;      // NEMA17 motor shaft exact diameter = 5
pulley_t_ht = 12;       // length of toothed part of pulley, standard = 12
pulley_b_ht = 20;                // pulley base height, standard = 8. Set to same as idler_ht if you want an idler but no pulley.
pulley_b_dia = 20;      // pulley base diameter, standard = 20
no_of_nuts = 0;         // number of captive nuts required, standard = 1
nut_angle = 90;         // angle between nuts, standard = 90
nut_shaft_distance = 1.2;       // distance between inner face of nut and shaft, can be nega

holes = 7;
hole_diameter = 32;
hole_location = 0.3;

screws = 7;
screw_diameter = 4.2;
screw_location = 0.43;
screw_phase_offset = 0.5;


pulley_diameter = tooth_spacing(teeth,5,0.5715);
difference () {
  pulley ( "HTD 5mm" , pulley_diameter , 2.188 , 3.781, profile, teeth, motor_shaft, retainer, retainer_ht, idler, idler_ht );
  
  
  for ( i = [1:holes] )
    rotate([0, 0, i * (360/holes)])
      translate ([pulley_diameter * hole_location, 0, 0])
      {
        cylinder(h=20, r1=hole_diameter/2, r2=hole_diameter/2);
      }

  for ( i = [1:screws] )
    rotate([0, 0, (i + screw_phase_offset) * (360/holes)])
      translate ([pulley_diameter * screw_location, 0, 0])
      {
        cylinder(h=20, r1=screw_diameter/2, r2=screw_diameter/2);
      }
}
