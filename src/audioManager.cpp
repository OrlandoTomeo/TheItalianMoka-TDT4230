#include "audioManager.h"
#define DR_WAV_IMPLEMENTATION
#include "utils/dr_wav.h"
#include <iostream>

namespace SpaceEngine
{
    void AudioManager::Initialize()
    {
        m_device = alcOpenDevice(nullptr);
        if (!m_device) {
            std::cerr << "Errore: Impossibile aprire il device audio!" << std::endl;
            return;
        }

        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context || !alcMakeContextCurrent(m_context)) {
            std::cerr << "Errore: Impossibile creare il contesto OpenAL!" << std::endl;
            return;
        }

        alGenSources(1, &m_musicSource);
    }

    void AudioManager::Shutdown()
    {
        alSourceStop(m_musicSource);
        alDeleteSources(1, &m_musicSource);

        for (ALuint source : m_sources) {
            alSourceStop(source);
            alDeleteSources(1, &source);
        }
        m_sources.clear();

        for (auto& pair : m_soundBuffers) {
            alDeleteBuffers(1, &pair.second);
        }
        m_soundBuffers.clear();

        alcMakeContextCurrent(nullptr);
        if (m_context) alcDestroyContext(m_context);
        if (m_device) alcCloseDevice(m_device);
    }

    void AudioManager::LoadSound(const std::string& name, const std::string& filePath)
    {
        if (m_soundBuffers.find(name) != m_soundBuffers.end()) {
            return;
        }

        unsigned int channels;
        unsigned int sampleRate;
        drwav_uint64 totalPCMFrameCount;
        
        short* pSampleData = drwav_open_file_and_read_pcm_frames_s16(
            filePath.c_str(), 
            &channels, 
            &sampleRate, 
            &totalPCMFrameCount, 
            nullptr
        );

        if (pSampleData == nullptr) {
            std::cerr << "Errore: Impossibile caricare il file audio: " << filePath << std::endl;
            return;
        }

        ALenum format;
        if (channels == 1) format = AL_FORMAT_MONO16;
        else if (channels == 2) format = AL_FORMAT_STEREO16;
        else {
            drwav_free(pSampleData, nullptr);
            return;
        }

        ALuint bufferID;
        alGenBuffers(1, &bufferID);
        alBufferData(bufferID, format, pSampleData, (ALsizei)(totalPCMFrameCount * channels * sizeof(short)), sampleRate);

        drwav_free(pSampleData, nullptr);

        m_soundBuffers[name] = bufferID;
    }

    ALuint AudioManager::GetAvailableSource()
    {
        for (ALuint source : m_sources) {
            ALint state;
            alGetSourcei(source, AL_SOURCE_STATE, &state);
            if (state != AL_PLAYING) {
                return source;
            }
        }

        ALuint newSource;
        alGenSources(1, &newSource);
        m_sources.push_back(newSource);
        return newSource;
    }

    void AudioManager::PlaySound(const std::string& name)
    {
        if (m_soundBuffers.find(name) == m_soundBuffers.end()) return;

        ALuint buffer = m_soundBuffers[name];
        ALuint source = GetAvailableSource();

        alSourcei(source, AL_BUFFER, buffer);
        alSourcei(source, AL_LOOPING, AL_FALSE); 
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef(source, AL_GAIN, 1.0f); 
        alSourcePlay(source);
    }

    ALuint AudioManager::PlaySoundLoop(const std::string& name, float volume)
    {
        if (m_soundBuffers.find(name) == m_soundBuffers.end()) return 0;

        ALuint buffer = m_soundBuffers[name];
        ALuint source = GetAvailableSource();

        alSourcei(source, AL_BUFFER, buffer);
        alSourcei(source, AL_LOOPING, AL_TRUE);
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef(source, AL_GAIN, volume);
        alSourcePlay(source);

        return source;
    }

    void AudioManager::PlayMusic(const std::string& name, bool loop)
    {
        if (m_soundBuffers.find(name) == m_soundBuffers.end()) return;

        alSourceStop(m_musicSource);

        ALuint buffer = m_soundBuffers[name];
        alSourcei(m_musicSource, AL_BUFFER, buffer);
        alSourcei(m_musicSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcei(m_musicSource, AL_SOURCE_RELATIVE, AL_TRUE);
        alSourcef(m_musicSource, AL_GAIN, m_musicVolume); // Usa il volume dedicato alla musica
        alSourcePlay(m_musicSource);
    }

    void AudioManager::StopMusic()
    {
        alSourceStop(m_musicSource);
    }

    void AudioManager::SetVolume(float volume)
    {
        m_masterVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f ? 1.0f : volume);
        
        alListenerf(AL_GAIN, m_masterVolume);
    }

    void AudioManager::SetMusicVolume(float volume)
    {
        m_musicVolume = (volume < 0.0f) ? 0.0f : (volume > 1.0f ? 1.0f : volume);
        
        alSourcef(m_musicSource, AL_GAIN, m_musicVolume);
    }
}