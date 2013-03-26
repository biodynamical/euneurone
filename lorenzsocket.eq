% Lorenz set of equations

<Parameters>
sigma=10
beta=2.666666667
rho=28
chi=250
xi=250
% f1=0.75
% f2=0.75
% f3=0.75
f4a1=-10
f4a2=0
f4b1=-10
f4b2=0
f4c1=-10
f4c2=0
l1=-1
l2=-1
l3=-1
l4a=-1
l4b=-1
l4c=-1
m4a=1
m4b=1
m4c=1
r1=0.75
pp1=-1
pp2=-1
pp3=-1
pp4a=-0.1
pp4b=-3
pp4c=-2
sc=500
sc1=800
sc2=10000
sc3=10000
sc4=70000
v=0.5
rc=1
ad1=0
ad2=0
ad31=-0.05
ad32=-0.05

<Equation>
to=pulse(t,chi,xi)
% Rate control
qx=(x/(x+sc))
qy=(y)/(y+sc)
qz=(z)/(z+sc)
q4a1=qx+ad1
s4a1=f4a1*(exp(pp4a*q4a1)+l4a)+m4a
q4a2=qy+ad1
s4a2=f4a2*(exp(pp4a*q4a2)+l4a)+m4a
%===
q4b1=qx*qz+ad2
s4b1=f4b1*(exp(pp4b*q4b1)+l4b)+m4b
q4b2=qx+ad2
s4b2=f4b2*(exp(pp4b*q4b2)+l4b)+m4b
%===
q4c1=qx*qy+ad31
s4c1=f4c1*(exp(pp4c*q4c1)+l4c)+m4c
q4c2=qz+ad32
s4c2=f4c2*(exp(pp4c*q4c2)+l4c)+m4c
% Lorenz
x'=(-sigma*x*s4a1+sigma*y*s4a2)
y'=((-x*z)*s4b1+s4b2*(rho*x)-y)
z'=((x*y)*s4c1-(beta*z)*s4c2)
t'=1

<Initial>
x=5
y=5
z=1
t=0
s4a1=1
s4a2=1
s4b1=1
s4c1=1
s4b2=1
s4c2=1
q4a1=0
q4a2=0
q4b1=0
q4b2=0
q4c1=0
q4c2=0
qx=0
qy=0
qz=0
to=0

<Options>
Duration=499
Output=1000
Step=0.01
Integrator=Runge-Kutta
Graph=Evolution,x,y,z
Graph=x,y
Graph=Evolution,qx,qy,qz
Graph=Evolution,s4a1,s4a2
Graph=Evolution,s4b1,s4b2
Graph=Evolution,s4c1,s4c2
Socket=5555
Client=Sinewave,1,1000

<Name>
Lorenzsocket.eq

<Description>
Lorenz' set of equations with rate control
(gain control)

