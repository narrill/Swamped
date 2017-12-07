#include "ParticleSystem.h"
#include "Game.h"
#include "GlobalFunctions.h"

ParticleSystem::ParticleSystem(unsigned int maxParticles, float particleLifeTime) {
	m_bounds.m_max = XMFLOAT3(100, 100, 100);
	m_bounds.m_min = XMFLOAT3(-100, 0, -100);
	m_particlesPerSecond = maxParticles / particleLifeTime;
	m_particles.resize(maxParticles);
	m_lifeTime = particleLifeTime;
}

ParticleSystem::ParticleSystem() : ParticleSystem(200000, 20) {

}

ParticleSystem::~ParticleSystem() {

}

size_t ParticleSystem::GetParticleCount() {
	return m_particleCount;
}

vector<Particle> & ParticleSystem::GetParticles() {
	return m_particles;
}

float ParticleSystem::GetLifeTime()
{
	return m_lifeTime;
}

void ParticleSystem::Update(Game * g, float dt, float totalTime) {
	StartTimer();
	//generate new particles
	unsigned int newParticles = m_particlesPerSecond * dt;
	for (unsigned int c = 0; c < newParticles; c++) {
		if (m_particleIndex >= m_particles.size())
			m_particleIndex = 0;
		Particle & p = m_particles[m_particleIndex];
		p.m_startPosition = XMFLOAT3(fRand(-100, 100), fRand(0, 100), fRand(-100, 100));
		p.m_velocity = XMFLOAT3(fRand(-1, 1), fRand(-2, -1), fRand(-1, 1));
		p.m_birthTime = totalTime;
		float size = fRand(.1f, .15f);
		p.m_size = size;
		++m_particleIndex;
		if (m_particleCount < m_particles.size())
			++m_particleCount;
	}

	StopTimer();
}