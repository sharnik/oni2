open Revery;
open Revery.UI;
open Oni_Core;
open Oni_Model;
open Oni_Components;

module Colors = Feature_Theme.Colors;

module Constants = {
  let menuWidth = 400;
  let menuHeight = 320;
};

module Styles = {
  open Style;

  let container = theme => [
    backgroundColor(Colors.Menu.background.from(theme)),
    color(Colors.Menu.foreground.from(theme)),
    width(Constants.menuWidth),
  ];

  let inputContainer = [padding(5)];

  let input = font => [
    border(~width=2, ~color=Color.rgba(0., 0., 0., 0.1)),
    backgroundColor(Color.rgba(0., 0., 0., 0.3)),
    color(Revery.Colors.white),
    fontFamily(font),
    fontSize(14.),
  ];

  let dropdown = [height(Constants.menuHeight), overflow(`Hidden)];

  let menuItem = [fontSize(14.), cursor(Revery.MouseCursors.pointer)];

  let label = (~font: UiFont.t, ~theme, ~highlighted) => [
    fontFamily(highlighted ? font.fontFileSemiBold : font.fontFile),
    textOverflow(`Ellipsis),
    fontSize(12.),
    color(
      highlighted
        ? Colors.Oni.normalModeBackground.from(theme)
        : Colors.Menu.foreground.from(theme),
    ),
    textWrap(TextWrapping.NoWrap),
  ];

  let progressBarTrack = [height(2), overflow(`Hidden)];

  let progressBarIndicator = (~width, ~offset, ~theme) => [
    height(2),
    Style.width(width),
    transform(Transform.[TranslateX(offset)]),
    backgroundColor(Colors.Oni.normalModeBackground.from(theme)),
  ];
};

let onFocusedChange = index =>
  GlobalContext.current().dispatch(ListFocus(index));

let onInputClicked = selection =>
  GlobalContext.current().dispatch(QuickmenuInputClicked(selection));

let onSelect = _ => GlobalContext.current().dispatch(ListSelect);

let progressBar = (~progress, ~theme, ()) => {
  let indicator = () => {
    let width = int_of_float(float(Constants.menuWidth) *. progress);
    let offset = 0.;

    <View style={Styles.progressBarIndicator(~width, ~offset, ~theme)} />;
  };

  <View style=Styles.progressBarTrack> <indicator /> </View>;
};

let%component busyBar = (~theme, ()) => {
  let%hook (time, _, _) =
    Animation.(
      animate(Time.ms(1500))
      |> ease(Easing.easeInOut)
      |> repeat
      |> tween(0., 1.)
      |> Hooks.animation
    );

  let indicator = () => {
    let width = 100;
    let trackWidth = float(Constants.menuWidth + width);
    let offset = trackWidth *. Easing.easeInOut(time) -. float(width);

    <View style={Styles.progressBarIndicator(~width, ~offset, ~theme)} />;
  };

  <View style=Styles.progressBarTrack> <indicator /> </View>;
};

let make =
    (
      ~font: UiFont.t,
      ~theme,
      ~configuration: Configuration.t,
      ~state: Quickmenu.t,
      ~placeholder: string="type here to search the menu",
      ~onFocusedChange: int => unit=onFocusedChange,
      ~onSelect: int => unit=onSelect,
      (),
    ) => {
  let Quickmenu.{
        items,
        filterProgress,
        ripgrepProgress,
        focused,
        query,
        selection,
        prefix,
        variant,
        _,
      } = state;

  let progress =
    Actions.(
      switch (filterProgress, ripgrepProgress) {
      | (Loading, _)
      | (_, Loading) => Loading

      | (InProgress(a), InProgress(b)) => InProgress((a +. b) /. 2.)

      | (InProgress(value), _)
      | (_, InProgress(value)) => InProgress(value)

      | (Complete, Complete) => Complete
      }
    );

  let renderItem = index => {
    let item = items[index];
    let isFocused = Some(index) == focused;

    let style = Styles.label(~font, ~theme);
    let text = Quickmenu.getLabel(item);
    let highlights = item.highlight;
    let normalStyle = style(~highlighted=false);
    let highlightStyle = style(~highlighted=true);
    let labelView =
      <HighlightText style=normalStyle highlightStyle text highlights />;

    <MenuItem
      onClick={() => onSelect(index)}
      theme
      style=Styles.menuItem
      label={`Custom(labelView)}
      icon={item.icon}
      font
      onMouseOver={() => onFocusedChange(index)}
      isFocused
    />;
  };

  let input = () =>
    <View style=Styles.inputContainer>
      <Input
        placeholder
        ?prefix
        cursorColor=Revery.Colors.white
        style={Styles.input(font.fontFile)}
        isFocused=true
        onClick=onInputClicked
        value=query
        selection
      />
    </View>;

  let dropdown = () =>
    <View style=Styles.dropdown>
      <FlatList rowHeight=40 count={Array.length(items)} focused>
        ...renderItem
      </FlatList>
      {switch (progress) {
       | Complete => <progressBar progress=0. theme /> // TODO: SHould be REact.empty, but a reconciliation bug then prevents the progress bar from rendering
       | InProgress(progress) => <progressBar progress theme />
       | Loading => <busyBar theme />
       }}
    </View>;

  <AllowPointer>
    <OniBoxShadow configuration theme>
      <View style={Styles.container(theme)}>
        {switch (variant) {
         | EditorsPicker => React.empty
         | _ => <input />
         }}
        {switch (variant) {
         | Wildmenu(SearchForward | SearchReverse) => React.empty
         | _ => <dropdown />
         }}
      </View>
    </OniBoxShadow>
  </AllowPointer>;
};
