import React from 'react';
import { smPrepare, smDiscuss } from './sm';
import download from './download';

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
    obj.forEach((o) => o.remove());
  }

  handleChange = ({ target }) => {
    this.setState({ input: target.value });
  };

  handleSubmit = (e) => {
    e.preventDefault();
    const { input } = this.state;
    this.progress(input);
  };

  progress = (input) => {
    const { kink } = this.props;
    const { obj, type } = this.state;
    switch (type) {
      case 0: {
        const res = kink ? smPrepare(input, true) : [parseInt(input, 10)];
        const os = [];
        res.forEach((v) => {
          os.push(new Module.Bob4(v));
        });
        this.setState({
          obj: os,
          type: 1,
          input: '',
          output: '',
        });
        break;
      }
      case 1: {
        let str = input;
        let output = '';
        obj.forEach((o) => {
          output += o.inquiry(str.substr(0, Module.garbleSize4 * 2));
          str = str.substr(Module.garbleSize4 * 2);
        });
        this.setState({
          type: 2,
          input: '',
          output,
        });
        break;
      }
      case 2:
        this.setState({
          type: 3,
          input: '',
          output: '',
        });
        break;
      case 3: {
        let str = input;
        const output = [];
        obj.forEach((o) => {
          let r = o.evaluate(str.substr(0, Module.receiveSize4 * 2));
          if (r === -1)
            r = NaN;
          output.push(r);
          str = str.substr(Module.receiveSize4 * 2);
        });
        this.setState({
          type: 4,
          input: '',
          output: kink ? smDiscuss(output) : output[0],
        });
        break;
      }
      default:
        break;
    }
  };

  handleUpload = ({ target: { files: [f] } }) => {
    if (!f) {
      return;
    }
    const fr = new FileReader();
    fr.onload = ({ target: { result } }) => {
      this.progress(result);
    };
    fr.readAsText(f);
  };

  handleDownload = () => {
    const { type, output } = this.state;
    download(`${type}-bob-to-alice`, output);
  };

  render() {
    const { kink } = this.props;
    const { type, input, output } = this.state;

    const prompt = kink ? (
      <h2>What&apos;s your base64 string?</h2>
    ) : (
      <h2>What&apos;s your number? (0, 1, 2, or 3 only)</h2>
    );

    const warn = kink ? (
      <p className="warn">Warning: it may take 1 seconds to 1 minute to complete the computation. PLEASE BE PATIENT.</p>
    ) : undefined;

    switch (type) {
      case 0:
        return (
          <form onSubmit={this.handleSubmit}>
            {prompt}
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
          </form>
        );
      case 1:
      case 3:
        return (
          <form onSubmit={this.handleSubmit}>
            <h2>Get a message from Alice and paste it here.</h2>
            <input type="text" value={input} onChange={this.handleChange} />
            <input type="submit" value="Next" />
            <p>--Or--</p>
            <input type="file" onChange={this.handleUpload} />
            {warn}
          </form>
        );
      case 2:
        return (
          <form onSubmit={this.handleSubmit}>
            <h2>Send the following message to Alice.</h2>
            <input type="button" value="Download" onClick={this.handleDownload} />
            <pre>{output}</pre>
            <input type="submit" value="Next" />
          </form>
        );
      default:
        return (
          <form>
            <h2>Result obtained. Share with Alice.</h2>
            <p>The minimum of your number and Alice&apos;s number is:</p>
            <pre>{output}</pre>
          </form>
        );
    }
  }
}
