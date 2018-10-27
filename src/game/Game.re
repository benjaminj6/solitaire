/* type cardList = list(card); */
open Types;

type state = {
  location,
  move,
  moves,
  moveKey,
};

type action =
  | UpdateMove(move)
  | MoveCard(move)
  | Undo
  | DealHand
  | Init;

let component = ReasonReact.reducerComponent("Game");

/* Namespaced Modules */
module Utils = Game_Utils;
module Styles = Game_Styles;

/* Handlers */
let getNextMove = (updater, state, ~location, ~card: option(card)) => {
  let {moveKey, move} = state;

  switch (moveKey) {
  | Prev => updater(UpdateMove({prev: Some(location), next: None, card}))
  | Next => updater(MoveCard({...move, next: Some(location)}))
  };
};

/* STATE */
let initialState = {
  location: {
    foundation: [|[], [], [], []|],
    tableau: [||],
    stock: [],
    hand: [],
  },
  move: {
    prev: None,
    next: None,
    card: None,
  },
  moveKey: Prev,
  moves: [],
};

/* COMPONENT */
let make = _children => {
  ...component,
  initialState: () => initialState,
  reducer: (action, state) =>
    switch (action) {
    | Init =>
      let (tableau, stock) =
        Utils.generateDeck()
        |> Utils.shuffleDeck(Js.Math.random())
        |> Utils.dealCards;

      ReasonReact.Update({
        ...initialState,
        location: {
          ...initialState.location,
          tableau,
          stock,
        },
      });
    | DealHand =>
      /* Go grab the first 3 from the list */
      let {stock, hand} = state.location;
      let prepareHand = (listA, listB) => {
        /* Flip all the cards in listA */
        let flippedListA =
          List.mapi(
            (i, card) => {...card, faceUp: true, selectable: i == 0},
            listA,
          );
        List.append(flippedListA, listB);
      };

      let prepareStock = list =>
        List.rev_map(
          card => {...card, faceUp: false, selectable: false},
          list,
        );

      let (nextHand, rest) =
        switch (stock) {
        /* NOTE -- move all of hand back into stock. Will need to add logic when limiting the number of deals */
        | [] => ([], prepareStock(hand))
        | [a] => (prepareHand([a], hand), [])
        | [a, b] => (prepareHand([b, a], hand), [])
        | [a, b, c, ...rest] => (prepareHand([c, b, a], hand), rest)
        };

      let move =
        List.length(stock) == 0 ?
          {prev: Some(Hand), next: Some(Stock), card: None} :
          {prev: Some(Stock), next: Some(Hand), card: None};

      ReasonReact.Update({
        ...state,
        moves: [move, ...state.moves],
        location: {
          ...state.location,
          hand: nextHand,
          stock: rest,
        },
      });
    | UpdateMove(move) => ReasonReact.Update({...state, move, moveKey: Next})
    /* TODO -- undo whatever the last move was */
    | Undo =>
      let {moves} = state;
      
      let nextState =
        switch (moves) {
        | [] => state.location
        | [lastMove, ..._rest] => Undo.undoMove(lastMove, state.location)
        };

      ReasonReact.Update({ ...state, location: nextState });
    | MoveCard({prev, next, card}) =>
      let (location, wasValidMove) =
        switch (prev, next, card) {
        | (Some(prevLocation), Some(nextLocation), Some(card)) =>
          Moves.getUpdatedLocation(
            ~prevLocation,
            ~nextLocation,
            ~card,
            ~location=state.location,
          )
        | _ => (state.location, false)
        };

      ReasonReact.Update({
        location,
        move: initialState.move,
        moveKey: initialState.moveKey,
        moves:
          wasValidMove ? [{prev, next, card}, ...state.moves] : state.moves,
      });
    },
  render: self => {
    let onClickCard = getNextMove(self.send, self.state);

    <div className=Styles.container>
      <header className=Styles.header>
        <button className=Styles.newGame onClick={_ev => self.send(Init)}>
          {ReasonReact.string("New Game")}
        </button>
        <button className=Styles.newGame onClick={_ev => self.send(Undo)}>
          {ReasonReact.string("Undo")}
        </button>
      </header>
      <div className=Styles.grid>
        <Foundation rows={self.state.location.foundation} onClickCard />
        <div className={Emotion.css("grid-column: 5 / 7")}>
          <Hand onClickCard hand={self.state.location.hand} />
        </div>
        <Stock
          cards={self.state.location.stock}
          onClick={() => self.send(DealHand)}
        />
        <Tableau
          rows={self.state.location.tableau}
          onClickCard={i => onClickCard(~location=Tableau(i))}
        />
      </div>
    </div>;
  },
};