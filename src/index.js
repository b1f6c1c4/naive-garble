import React from 'react';
import { render } from 'react-dom';
import Alice from './Alice';
import Bob from './Bob';
import './index.css';

class App extends React.PureComponent {
  state = {
    type: null,
  };

  handleAlice = () => {
    this.setState({ type: 'alice' });
  };

  handleBob = () => {
    this.setState({ type: 'bob' });
  };

  render() {
    const { type } = this.state;
    switch (type) {
      case 'alice':
        return (
          <Alice />
        );
      case 'bob':
        return (
          <Bob />
        );
      default:
        return (
          <React.Fragment>
            <p>Pick one from below.</p>
            <div>
              <button type="button" onClick={this.handleAlice}>I&apos;m Alice</button>
              <button type="button" onClick={this.handleBob}>I&apos;m Bob</button>
            </div>
          </React.Fragment>
        );
    }
  }
}

render((
  <App />
), document.getElementById('app'));
