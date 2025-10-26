from google import genai
from google.genai import types
import os
import time

import openai

from dotenv import load_dotenv
load_dotenv()

class Gemini:
    def __init__(self, model_name="gemini-2.5-pro", system_prompt=None):
        self.client = genai.Client(api_key=os.getenv("GEMINI_API_KEY"))
        self.model_name = model_name
        self.system_prompt = system_prompt

        # self.config = types.GenerateContentConfig(
        #     system_instruction=self.system_prompt,
        #     response_mime_type="application/json",
        #     response_schema=ConstituentDocuments
        #     # response_schema=list[ConstituentDocuments]
        # )

        self.model_costs = {
            "gemini-2.5-flash-lite": ...,
            "gemini-2.5-flash": ...,
            "gemini-2.5-pro": ...,
        }

    def send_message(self, user_prompt: str, temperature: float=0.2) -> tuple[types.GenerateContentResponse, str]:
        """Returns the full response object and the text content."""
        max_retries = 3
        while max_retries > 0:
            try:
                response = self.client.models.generate_content(
                    model=self.model_name,
                    contents=user_prompt,
                    config=types.GenerateContentConfig(
                        thinking_config=types.ThinkingConfig(
                        include_thoughts=True,
                        thinking_budget=-1
                        ),
                        temperature=temperature,
                    )
                )
                break
            except Exception as e:
                print(f"Error generating content: {e}")
                print("Retrying in 30 seconds as it may be a rate limit issue...")
                time.sleep(30)
                max_retries -= 1
                if max_retries == 0:
                    raise e

        return response, response.candidates[0].content.parts[-1].text
    
class OpenAI:
    def __init__(self, model_name="gpt-4", system_prompt=""):
        self.client = openai.OpenAI(
            api_key=os.getenv("AI_GATEWAY_API_KEY"),
            base_url=os.getenv("AI_GATEWAY_BASE_URL"),
        )

        self.system_prompt = system_prompt
        self.model_name = model_name

    def send_message(self, user_prompt: str, temperature: float=0.2):
        """Returns the full response object and the text content."""
        max_retries = 3
        while max_retries > 0:
            try:
                response = self.client.chat.completions.create(
                    model=self.model_name,
                    messages = [
                        {
                            "role": "system",
                            "content": self.system_prompt
                        },
                        {
                            "role": "user",
                            "content": user_prompt
                        }
                    ],
                    temperature=temperature,
                    reasoning_effort="medium",
                )
                break
            except Exception as e:
                print(f"Error generating content: {e}")
                print("Retrying in 30 seconds as it may be a rate limit issue...")
                time.sleep(30)
                max_retries -= 1
                if max_retries == 0:
                    raise e

        message = response.choices[0].message
        llm_response_text = getattr(message, "reasoning_content", None) or getattr(message, "content", None)
        return response, llm_response_text
        # return response, response.choices[0].message.content