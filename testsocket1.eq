<Parameters>
a=1
%chi=10
%xi=10

<Equation>
%to=pulse(t,chi,xi)
x=socket(1,1)
y'=pow(x,1.5)
t'=1

<Initial>
x=0
y=1
t=0

<Options>
Duration=100
Output=1000
Step=0.01
Integrator=Runge-Kutta
Graph=Evolution,x,y
Socket=5555
Client=Sinewave,1,1000

<Name>
Testsocket1.eq

<Description>
Test socket with sine wave
