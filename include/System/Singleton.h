/**********************************
*        Bomberman3D Engine       *
*  Created by: Cmaranec (Kennny)  *
**********************************/
// Singleton definition

#ifndef BOMB_SINGLETON_H
#define BOMB_SINGLETON_H

#include <assert.h>

template <class T>
class Singleton
{
    public:
        static T* instance()
        {
            if (!m_instance)
                m_instance = new T;

            assert(m_instance != NULL);

            return m_instance;
        }

    private:
        static T* m_instance;
};

template <class T> T* Singleton<T>::m_instance = NULL;

/** \class Singleton
 *  \brief Trida singletonu
 *
 * Singleton je takovy stavebni prvek aplikace, ktery zaruci jen jednu instanci dane tridy po celou dobu behu aplikace
 *
 * Pote se vytvori makro pro referenci jednotlivych instanci trid pomoci #define sMojeTrida Singleton<MojeTrida>::instance()
 */

/** \var Singleton::m_instance
 *  \brief Ukazatel na tridu, kterou dany singleton instancuje
 */

/** \fn Singleton::instance
 *  \brief Funkce starajici se o vraceni ukazatele na instancovanou tridu
 *
 * Pokud dana instance tridy neexistuje, vytvori se. Pokud ale nebyla vytvorena ani tak, je nutne ukoncit beh programu - pravdepodobne se vyskytla chyba pri alokaci pameti
 */

#endif
