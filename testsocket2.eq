<Parameters>
a=1
b=2
%chi=10
%xi=10

<Equation>
%to=pulse(t,chi,xi)
x=socket(0,0)
y'=socket(1,0)
z'=a*x
p'=b*y
t'=1

<Initial>
x=0
y=0
z=1
p=1
t=0

<Options>
Duration=1000
Output=1000
Step=0.01
Integrator=Runge-Kutta
Graph=Evolution,x,y
Graph=Evolution,z,p
Socket=5556
Client=Sinewave,0,1,100000
Client=Cosinewave,1,1,100000

<Name>
Testsocket2.eq

<Description>
Test socket with sine wave
