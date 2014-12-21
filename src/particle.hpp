#ifndef _PARTICLE_H_
#define _PARTICLE_H_

#include <glm/glm.hpp>
#include <vector>

struct Particle{
	// Add a single force to the particle, this could
	//	 be anything from drag to the mouseforce applied
	//	 to each particle. Note that this is NOT setting
	//	 the speed of the particle.
    void addForce(glm::vec3 const& m_force)
    {
        force.push_back(m_force);
    }

	// Sums up the total force of the force vector and 
	//	 returns a vec3 of the result. This will be used
	//	 in calculating the position and speed of the 
	//	 particle by determing the total forces acting
	//	 on it.
    glm::vec3 getTotalForce() const
    {
        glm::vec3 total(0.0,0.0,0.0);
        for(auto i=0; i < force.size(); i++) {
            total += force[i];
        }
        return total;
    }

	// Clear force vector, essentially 'stopping' a particle
    void clearForce()
    {
        force.clear();
    }

	glm::vec3 pos, speed;            // position and speed of a particle represented as vec3's
    std::vector<glm::vec3> force;    // list of forces acting on a particle
    unsigned char r,g,b,a;           // specific color of particle
    float size;						 // how large each particle is
    float life;                      // remaining life of a particle
    float cameradistance;            // squared distance to camera : -1.0f if dead
};

#endif