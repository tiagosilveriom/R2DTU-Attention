//
// Created by s154250 on 9/15/19.
//
#pragma once

#define LANG_DANISH 0

#if LANG_DANISH

#define L10N_HELLO "Goddag"
#define L10N_MY_NAME "Mit navn er Ar Tu \\pau=000\\ Di Ti Ju"
#define L10N_YOUR_NAME "Hvad hedder du?"
#define L10N_NICE_TO_MEET "Godt at møde dig %s"
#define L10N_SELF_INTRO "Jeg har lært. At sætte mig i andres sted. \\\\pau=750\\\\ Jeg gør det vedd at holde styr på. Hvad andre ved. Om verden. \\\\pau=750\\\\ På den måde. Kan jeg bestå en Sally-Annn test. \\\\pau=750\\\\ Jeg genkender ansigter. \\\\pau=750\\\\ Og jeg genkender. Dine bevægelser. \\\\pau=750\\\\ Og jeg holder styr på. Hvem der ser hvaddd."
#define L10N_LOOKING_FOR "Hvis du ledder efter den %s klods. \\pau=750\\ Så er den nu i kasse %s"
#define L10N_ANNOUNCE_LOCATION "Den %s klods er blevet flyttet til kasse %s"
#define L10N_EXPLAIN "%s har flyttet den %s klods til kasse %d \\pau=750\\ mens %s ikke var her."

#define L10N_RED "røde"
#define L10N_GREEN "grønne"
#define L10N_BLUE "blå"

#define L10N_SECOND_ORDER_BOX "%s tror at %s tror at den %s er i %s"
#define L10N_SECOND_ORDER_TABLE "%s tror at %s tror at den %s ikke er i en kasse"
#define L10N_FIRST_ORDER_BOX "%s tror at den %s er i %s"
#define L10N_FIRST_ORDER_TABLE "%s tror at den %s ikke er i en kasse"
#define L10N_ZEROTH_ORDER_BOX "Den %s er i %s"
#define L10N_ZEROTH_ORDER_TABLE "Den %s er ikke i nogen kasse"

#else

#define L10N_HELLO "Hello"
#define L10N_MY_NAME "My name is R2DTU"
#define L10N_YOUR_NAME "What is your name?"
#define L10N_NICE_TO_MEET "Nice to meet you %s"
#define L10N_SELF_INTRO "I have learned to put myself into the shoes of others. \\pau=750\\ I do this by keeping track of what others know about the world. \\pau=750\\ That way I can pass a Sally-Anne test. \\pau=750\\ I use facial recognition. \\pau=750\\ I recognize your movements. \\pau=750\\ And I keep track of who sees what."
#define L10N_LOOKING_FOR "If you are looking for the %s cube. \\pau=750\\ It is now in box %s"
#define L10N_ANNOUNCE_LOCATION "The %s cube has been moved to box %s"
#define L10N_EXPLAIN "%s has moved the %s cube to box %d \\pau=750\\ while %s was not present."
#define L10N__OBJECT_EXPLAIN "%s has moved the %s to box %d \\pau=750\\ while %s was not present."
#define L10N_OBJECT_LOOKING_FOR "If you are looking for the %s . \\pau=750\\ It is now in box %s"

#define L10N_RED "red"
#define L10N_GREEN "green"
#define L10N_BLUE "blue"

#define L10N_SECOND_ORDER_BOX "%s believes that %s believes that the %s is in %s"
#define L10N_SECOND_ORDER_TABLE "%s believes that %s believes that the %s is not in any box"
#define L10N_FIRST_ORDER_BOX "%s believes that the %s is in %s"
#define L10N_FIRST_ORDER_TABLE "%s believes that the %s is not in any box"
#define L10N_ZEROTH_ORDER_BOX "The %s is in %s"
#define L10N_ZEROTH_ORDER_TABLE "The %s is not in any box"

#endif



