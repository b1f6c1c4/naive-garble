import React from 'react';

const { Module } = window;

export default class Bob extends React.PureComponent {
  state = {
    obj: null,
    type: 0,
    input: '',
    output: '',
  };

  componentWillUnmount() {
    const { obj } = this.state;
    if (obj) {
      obj.remove();
    }
  }

  handleChange = ({ target }) => {
    this.setState({ input: target.value });
  };

  handleSubmit = (e) => {
    e.preventDefault();
    const { obj, type, input } = this.state;
    switch (type) {
      case 0: {
        const v = parseInt(input, 10);
        const o = new Module.Bob4(v);
        this.setState({
          obj: o,
          type: 1,
          input: '',
          output: '',
        });
        break;
      }
      case 1:
        this.setState({
          type: 2,
          input: '',
          output: obj.inquiry(input),
        });
        break;
      case 2:
        this.setState({
          type: 3,
          input: '',
          output: '',
        });
        break;
      case 3: {
        let o = obj.evaluate(input);
        if (o === -1) {
          o = 'ERROR';
        }
        this.setState({
          type: 4,
          input: '',
          output: o,
        });
        break;
      }
      default:
        break;
    }
  };

  render() {
    const { type, input, output } = this.state;

    switch (type) {
      case 0:
        return (
          <form onSubmit={this.handleSubmit}>
            <p>What&apos;s your number? (0, 1, 2, or 3 only)</p>
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
          </form>
        );
      case 1:
      case 3:
        return (
          <form onSubmit={this.handleSubmit}>
            <p>Get a message from Alice and paste it here.</p>
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
          </form>
        );
      case 2:
        return (
          <form onSubmit={this.handleSubmit}>
            <p>Send the following message to Alice.</p>
            <pre>{output}</pre>
            <input type="submit" value="Next" />
          </form>
        );
      default:
        return (
          <form>
            <p>The minimum of your number and Alice&apos;s number is:</p>
            <pre>{output}</pre>
          </form>
        );
    }
  }
}
