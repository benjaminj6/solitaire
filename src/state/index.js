import deck from './deck'
import foundation from './foundation'

// Provides the initial state / the store structure for the app
const state = {
  deck,
  foundation
  /**
   * foundation:
   *    hearts: Array<card.id>
   *    clubs: Array<card.id>
   *    diamonds: Array<card.id>
   *    spades: Array<card.id>
   *
   * draw: Array<card.id>
   * waste: Array<card.id>
   */
}

export default state