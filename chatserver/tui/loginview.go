package tui

import "github.com/marcusolsson/tui-go"

type LoginHandler func(string)

type LoginView struct {
	tui.Box
	frame        *tui.Box
	LoginHandler LoginHandler
}

func NewLoginView() *LoginView {
	// https://github.com/marcusolsson/tui-go/blob/master/example/login/main.go
	user := tui.NewEntry()
	user.SetFocused(true)
	user.SetSizePolicy(tui.Maximum, tui.Maximum)

	label := tui.NewLabel("Enter your name: ")
	user.SetSizePolicy(tui.Expanding, tui.Maximum)

	userBox := tui.NewHBox(
		label,
		user,
	)
	userBox.SetBorder(true)
	userBox.SetSizePolicy(tui.Expanding, tui.Maximum)

	view := &LoginView{}
	view.frame = tui.NewVBox(
		tui.NewSpacer(),
		tui.NewPadder(-4, 0, tui.NewPadder(4, 0, userBox)),
		tui.NewSpacer(),
	)
	view.Append(view.frame)

	user.OnSubmit(func(e *tui.Entry) {
		if e.Text() != "" {
			if view.LoginHandler != nil {
				view.LoginHandler(e.Text())
			}
			e.SetText("")
		}
	})
	return view
}

func (v *LoginView) OnLogin(handler LoginHandler) {
	v.LoginHandler = handler
}
