#include "Particle.h"

Particle::Particle()
{
	m_mass = 50;
	m_position = Vector2f(50,50);
	m_speed = Vector2f(0,0);
	m_acceleration = Vector2f(0,0);
	m_movable = true;
}

Particle::Particle(float mass, Vector2f position, Vector2f speed)
{
	m_mass = mass;
	m_position = position;
	m_speed = speed;
}

//return mass of particle
float Particle::getMass() const
{
	return m_mass;
}

//set mass
void Particle::setMass(float const& mass)
{
	m_mass = mass;
}

//return the force vector
vector<Vector2f> Particle::getForce() const
{
	return m_force;
}

//return the total force of particle
Vector2f Particle::getTotalForce() const
{
	Vector2f totalForce(0,0);
	for(size_t i=0; i < m_force.size(); i++) {
		totalForce += m_force[i];
	}
	return totalForce;
}

void Particle::addForce(Vector2f const& force)
{
	m_force.push_back(force);
}

void Particle::clearForce()
{
	m_force.clear();
}

Vector2f Particle::getPosition() const
{
	return m_position;
}

void Particle::setPosition(Vector2f const& position)
{
	m_position = position;
}

Vector2f Particle::getSpeed() const
{
	return m_speed;
}

void Particle::setSpeed(Vector2f const& speed)
{
	m_speed = speed;
}

Vector2f Particle::getAcceleration() const
{
	return m_acceleration;
}

void Particle::setAcceleration(Vector2f const& acceleration)
{
	m_acceleration - acceleration;
}

void Particle::setStatic()
{
	m_movable = false;
}

void Particle::setMovable()
{
	m_movable = true;
}

void Particle::updatePosition(float const& dt)
{
	Vector2f previousPosition = m_position; //if x is pos, then store x(t)
	//from the gool ol' phsyics equations:
	//x(t+dt) = (1/2)*(F/m)*dt^2 + v(t)*dt + x(t)
	m_position = m_position + m_speed*dt + 0.5f*getTotalForce()/m_mass*(float)pow(dt,2);
	//update the speed by differentiating the position, now that we have both x and speed the particle moves
	m_speed = (m_position - previousPosition)/dt;

}
 