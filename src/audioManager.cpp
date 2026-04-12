#include "audioManager.h"
#define DR_WAV_IMPLEMENTATION
#include "utils/dr_wav.h"

namespace SpaceEngine
{
    void AudioManager::Initialize()
    {
        m_device = alcOpenDevice(nullptr);
        if (!m_device) {
            return;
        }

        m_context = alcCreateContext(m_device, nullptr);
        if (!m_context || !alcMakeContextCurrent(m_context)) {
            return;
        }

        alGenSources(1, &m_musicSource);
    }

    void AudioManager::Shutdown()
    {
        alSourceStop(m_musicSource);
        alDeleteSources(1, &m_musicSource);

        for (ALuint source : m_sources) {
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

        // Caricamento WAV con dr_wav
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
            return;
        }

        /*SPACE_ENGINE_INFO("File: {}", filePath);
        SPACE_ENGINE_INFO(" > Channels: {}", channels);         
        SPACE_ENGINE_INFO(" > SampleRate: {}", sampleRate);
        SPACE_ENGINE_INFO(" > Frames: {}", totalPCMFrameCount);per verificare che i file audio non siano vuoti o corrotti*/

        ALenum format; //se audio mono o stereo
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
        if (m_soundBuffers.find(name) == m_soundBuffers.end()) {
            return;
        }

        ALuint buffer = m_soundBuffers[name];
        ALuint source = GetAvailableSource();

        alSourcei(source, AL_BUFFER, buffer);
        alSourcei(source, AL_LOOPING, AL_FALSE); 
        alSourcei(source, AL_SOURCE_RELATIVE, AL_TRUE); //TODO: da cambiare se vogliamo suoni 3D(proiettili direzzionali, ecc)

        alSourcef(source, AL_GAIN, 1.0f);
        alSourcePlay(source);
    }

    void AudioManager::PlayMusic(const std::string& name, bool loop)
    {
        if (m_soundBuffers.find(name) == m_soundBuffers.end()) return;

        alSourceStop(m_musicSource);

        ALuint buffer = m_soundBuffers[name];
        alSourcei(m_musicSource, AL_BUFFER, buffer);
        alSourcei(m_musicSource, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
        alSourcei(m_musicSource, AL_SOURCE_RELATIVE, AL_TRUE); //TODO: da cambiare se vogliamo musica 3D(probabilmente no)
        alSourcef(m_musicSource, AL_GAIN, 1.f); // Volume musica al 50% di default
        alSourcePlay(m_musicSource);

        ALint state;
        alGetSourcei(m_musicSource, AL_SOURCE_STATE, &state);
    }

    void AudioManager::StopMusic()
    {
        alSourceStop(m_musicSource);
    }

    void AudioManager::SetVolume(float volume)
    {
        if (volume < 0.0f) volume = 0.0f;
        if (volume > 1.0f) volume = 1.0f;

        m_masterVolume = volume;
        alListenerf(AL_GAIN, volume);
    }
};