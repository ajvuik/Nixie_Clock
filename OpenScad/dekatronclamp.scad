$fa=0.5;
$fs=0.5;

clampsize=40;
clampthick=10;
dekatron=30;
M3=3.1;
M4=4.1;

module cutout(){
    union(){
        cylinder(r=dekatron/2,h=clampthick+2);
        translate([-0.5,dekatron/2-1,0])
            cube([1,13,clampthick+2]);
    }
}

module tab(){
    difference(){
        cube([5,7,10]);
        translate([-1,4,5]){
            rotate([0,90,0])
                cylinder(r=M3/2, h=7);
        }
    }
}

module mountholes(){
    translate([4,4,-1])
        cylinder(r=M3/2, h=clampthick+2);
    translate([4,36,-1])
        cylinder(r=M3/2, h=clampthick+2);
}

module maincube(){
    difference(){
        cube([clampsize,clampsize,clampthick]);
        mountholes();
    }
}

module mainshape(){
    maincube();
    translate([clampsize/2-2.5,clampsize,0])
       tab();
}

union(){
    difference(){
        mainshape();
        translate([clampsize/2,clampsize/2,-1])
            cutout();
    }
}