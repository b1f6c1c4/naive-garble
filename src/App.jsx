import React from 'react';
import Alice from './Alice';
import Bob from './Bob';

export default class App extends React.PureComponent {
  state = {
    type: null,
    kink: false,
  };

  handleAlice = () => {
    this.setState({ type: 'alice' });
  };

  handleBob = () => {
    this.setState({ type: 'bob' });
  };

  handleKAlice = () => {
    this.setState({ type: 'alice', kink: true });
  };

  handleKBob = () => {
    this.setState({ type: 'bob', kink: true });
  };

  render() {
    const { type, kink } = this.state;
    switch (type) {
      case 'alice':
        return (
          <Alice kink={kink} />
        );
      case 'bob':
        return (
          <Bob kink={kink} />
        );
      default:
        return (
          <React.Fragment>
            <h1>Introduction</h1>
            <p>
              <a href="https://en.wikipedia.org/wiki/Garbled_circuit">Garbled circuit</a>
                &nbsp;is a cryptography protocol that facilitates secured function evalutation (SFE).
                This site demonstrates the basics of it and can be used for SFE in limited scenarios (semi-trusted).
            </p>
            <h1>The simple demo</h1>
            <p>Pick one from below.</p>
            <div>
              <button type="button" onClick={this.handleAlice}>I&apos;m Alice</button>
              <button type="button" onClick={this.handleBob}>I&apos;m Bob</button>
            </div>
            <h1>
              From&nbsp;
              <a href="https://www.minegeck.net/lab/sm">sm contract</a>
              ?
            </h1>
            <div>
              <button type="button" onClick={this.handleKAlice}>I&apos;m Alice and I&apos;m kinky</button>
              <br />
              <button type="button" onClick={this.handleKBob}>I&apos;m Bob and I&apos;m kinky</button>
            </div>
            <h1>Source code?</h1>
            <p><a href="https://github.com/b1f6c1c4/naive-garble">Github</a></p>
          </React.Fragment>
        );
    }
  }
}
