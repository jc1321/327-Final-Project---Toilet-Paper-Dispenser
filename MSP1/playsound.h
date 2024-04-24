/*
 * playsound.h
 */

#ifndef PLAYSOUND_H
#define PLAYSOUND_H

void sound_init();

void PlaySound(int *Notes, int *Durations, int Length);

void PlayNote(int Note);

char PlaySoundISR();

void StopSong();

#endif /* PLAYSOUND_H */
