import React from 'react';
import { render } from 'react-dom';
import Alice from './Alice';
import Bob from './Bob';
import './index.css';

class App extends React.PureComponent {
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
                is a cryptography protocol that facilitates secured function evalutation (SFE).
                This site demonstrates the basics of it and can be used for SFE in limited scenarios (semi-trusted).
            </p>
            <h1>The simple demo</h1>
            <p>Pick one from below.</p>
            <div>
              <button type="button" onClick={this.handleAlice}>I&apos;m Alice</button>
              <button type="button" onClick={this.handleBob}>I&apos;m Bob</button>
            </div>
            <h1>
              From
              <a href="https://www.minegeck.net/lab/sm">sm contract</a>
              ?
            </h1>
            <div>
              <button type="button" onClick={this.handleKAlice}>I&apos;m Alice and I&apos;m kinky</button>
              <button type="button" onClick={this.handleKBob}>I&apos;m Bob and I&apos;m kinky</button>
            </div>
          </React.Fragment>
        );
    }
  }
}

render((
  <App />
), document.getElementById('app'));
