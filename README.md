# ContextFree
A new and wonderful [ossia score](https://ossia.io) add-on

This addon implements the [context-free art](https://www.contextfreeart.org) language as visual generator node.

It allows through a simple programming language to generate fancy recursive visuals: 

![example](https://www.contextfreeart.org/gallery/uploads//a9/58/a9585ee26b99b02fd19465902f2fd4d5//full_772.png?0) ((c) efi

```
startshape A

rule A {
20*{r 18}B{x 3}
B{s 3}
}

rule B {
75*{x .5 s 1 .9 r 4}C{a -.5 b .1 z 1}
}
rule B {
75*{x .5 s 1 .9 r -4}C{a -.5 b .1 z 1}
}

rule C {
20*{s .95}CIRCLE{ a -.8}
20*{s .95}TRIANGLE{ s .5 b 5 x 1 sat 1}
}
rule C .05 {
B{r 90}
}
rule C .05 {
B{r -90}
}
```
