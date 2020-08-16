$fa = 1;
$fs = 0.4;

//PCB Tray
cube([50,42,2]);
translate([14.097, 4.064])
cylinder(h=6,r=0.85);
translate([45.974,4.064])
cylinder(h=6,r=0.85);
translate([4.064,37.973])
cylinder(h=6,r=0.85);
translate([45.974,37.973])
cylinder(h=6,r=0.85);

difference(){
    translate([-2,0,0])
    cube([2,42,9]);
    translate([-2,4.5,5])
    cube([2,9.4,4]);
    translate([-2,18,5])
    cube([2,14,4]);
    translate([-0.2,0,2])
    cube([2,42,9]);
}
